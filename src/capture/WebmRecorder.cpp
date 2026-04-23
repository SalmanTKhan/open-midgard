#include "WebmRecorder.h"

#include "DebugLog.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace capture {

namespace {

constexpr size_t kQueueSlots = 4;

struct QueuedFrame {
    std::vector<uint8_t> rgba;  // tightly packed RGBA, inWidth*inHeight*4
    int64_t ptsMicros = 0;
    bool    valid = false;
};

void LogAvError(const char* where, int code)
{
    char buf[128] = {};
    if (av_strerror(code, buf, sizeof(buf)) < 0) {
        std::snprintf(buf, sizeof(buf), "(unknown code %d)", code);
    }
    DbgLog("[Capture][ffmpeg] %s failed: %s\n", where, buf);
}

}  // namespace

class WebmRecorderImpl {
public:
    ~WebmRecorderImpl() { Stop(); }

    bool Start(const std::filesystem::path& outPath,
               int inWidth, int inHeight, PixelFormat inFormat,
               int outWidth, int outHeight,
               int targetFps, int bitrateKbps)
    {
        if (m_running) return false;
        if (inWidth <= 0 || inHeight <= 0) return false;
        if (outWidth <= 0 || outHeight <= 0) return false;
        if (targetFps <= 0 || bitrateKbps <= 0) return false;

        m_inFormat = inFormat;
        m_bitrateKbps = bitrateKbps;

        // VP8 requires even output dimensions.
        outWidth  &= ~1;
        outHeight &= ~1;
        if (outWidth <= 0 || outHeight <= 0) return false;

        m_inWidth  = inWidth;
        m_inHeight = inHeight;
        m_outWidth  = outWidth;
        m_outHeight = outHeight;
        m_targetFps = targetFps;
        m_frameDurationUs = 1'000'000ULL / static_cast<uint64_t>(m_targetFps);
        m_firstPtsMicros = -1;
        m_lastAcceptedPtsUs = 0;
        m_encodedCount = 0;
        m_droppedCount = 0;
        m_queueFullCount = 0;
        m_encodeErrorCount = 0;

        const std::string outUtf8 = outPath.string();

        int rc = avformat_alloc_output_context2(&m_fmtCtx, nullptr, "webm", outUtf8.c_str());
        if (rc < 0 || !m_fmtCtx) {
            LogAvError("avformat_alloc_output_context2", rc);
            DestroyAll();
            return false;
        }

        const AVCodec* encoder = avcodec_find_encoder_by_name("libvpx");
        if (!encoder) encoder = avcodec_find_encoder(AV_CODEC_ID_VP8);
        if (!encoder) {
            DbgLog("[Capture] VP8 encoder unavailable (ffmpeg built without libvpx?)\n");
            DestroyAll();
            return false;
        }

        m_stream = avformat_new_stream(m_fmtCtx, nullptr);
        if (!m_stream) { DestroyAll(); return false; }

        m_codecCtx = avcodec_alloc_context3(encoder);
        if (!m_codecCtx) { DestroyAll(); return false; }

        m_codecCtx->codec_id   = AV_CODEC_ID_VP8;
        m_codecCtx->width      = m_outWidth;
        m_codecCtx->height     = m_outHeight;
        m_codecCtx->pix_fmt    = AV_PIX_FMT_YUV420P;
        // Millisecond time base. pts is computed from wall-clock microseconds
        // in the worker, so encoded duration matches real recording time even
        // when the capture rate is far below `targetFps`.
        m_codecCtx->time_base  = AVRational{ 1, 1000 };
        m_codecCtx->framerate  = AVRational{ m_targetFps, 1 };
        m_codecCtx->bit_rate   = static_cast<int64_t>(bitrateKbps) * 1000;
        m_codecCtx->gop_size   = m_targetFps * 5;
        m_codecCtx->thread_count = 0;
        // WebM muxer does not set AVFMT_GLOBALHEADER; avoid setting the
        // codec flag since it would make libvpx stash extradata that never
        // gets plumbed through to the muxed stream.

        // Realtime tuning. cpu-used range is -16..16; higher = faster.
        av_opt_set    (m_codecCtx->priv_data, "quality",         "realtime", 0);
        av_opt_set_int(m_codecCtx->priv_data, "cpu-used",        8, 0);
        av_opt_set_int(m_codecCtx->priv_data, "lag-in-frames",   0, 0);
        av_opt_set_int(m_codecCtx->priv_data, "error-resilient", 1, 0);
        av_opt_set    (m_codecCtx->priv_data, "deadline",        "realtime", 0);

        rc = avcodec_open2(m_codecCtx, encoder, nullptr);
        if (rc < 0) {
            LogAvError("avcodec_open2", rc);
            DestroyAll();
            return false;
        }

        rc = avcodec_parameters_from_context(m_stream->codecpar, m_codecCtx);
        if (rc < 0) {
            LogAvError("avcodec_parameters_from_context", rc);
            DestroyAll();
            return false;
        }
        m_stream->time_base = m_codecCtx->time_base;

        if (!(m_fmtCtx->oformat->flags & AVFMT_NOFILE)) {
            rc = avio_open(&m_fmtCtx->pb, outUtf8.c_str(), AVIO_FLAG_WRITE);
            if (rc < 0) {
                LogAvError("avio_open", rc);
                DestroyAll();
                return false;
            }
        }

        rc = avformat_write_header(m_fmtCtx, nullptr);
        if (rc < 0) {
            LogAvError("avformat_write_header", rc);
            DestroyAll();
            return false;
        }

        m_frame = av_frame_alloc();
        if (!m_frame) { DestroyAll(); return false; }
        m_frame->format = AV_PIX_FMT_YUV420P;
        m_frame->width  = m_outWidth;
        m_frame->height = m_outHeight;
        // Alignment 0 = libavutil picks the CPU-appropriate default.
        rc = av_frame_get_buffer(m_frame, 0);
        if (rc < 0) {
            LogAvError("av_frame_get_buffer", rc);
            DestroyAll();
            return false;
        }

        const AVPixelFormat srcPix = (m_inFormat == PixelFormat::Bgra8)
            ? AV_PIX_FMT_BGRA
            : AV_PIX_FMT_RGBA;
        m_sws = sws_getContext(m_inWidth, m_inHeight, srcPix,
                               m_outWidth, m_outHeight, AV_PIX_FMT_YUV420P,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!m_sws) {
            DbgLog("[Capture] sws_getContext returned null (in %dx%d out %dx%d)\n",
                m_inWidth, m_inHeight, m_outWidth, m_outHeight);
            DestroyAll();
            return false;
        }

        const size_t slotBytes = static_cast<size_t>(m_inWidth) * m_inHeight * 4;
        for (auto& slot : m_queue) {
            slot.rgba.assign(slotBytes, 0);
            slot.valid = false;
        }
        m_queueHead = 0;
        m_queueTail = 0;
        m_queueCount = 0;

        m_stopRequested.store(false);
        m_running = true;
        m_worker = std::thread([this] { WorkerLoop(); });

        DbgLog("[Capture] encoder ready: vp8 in=%dx%d out=%dx%d tb=1/%d bitrate=%lldkbps\n",
            m_inWidth, m_inHeight, m_outWidth, m_outHeight, m_targetFps,
            static_cast<long long>(bitrateKbps));
        return true;
    }

    bool PushFrame(const uint8_t* pixels, int stride, int64_t ptsMicros)
    {
        if (!m_running || !pixels) return false;
        if (stride < m_inWidth * 4) return false;

        if (m_firstPtsMicros < 0) {
            m_firstPtsMicros = ptsMicros;
        }
        const int64_t relUs = ptsMicros - m_firstPtsMicros;

        // Drop frames arriving faster than the target fps.
        if (m_lastAcceptedPtsUs > 0 &&
            relUs < static_cast<int64_t>(m_lastAcceptedPtsUs) +
                    static_cast<int64_t>(m_frameDurationUs / 2)) {
            m_droppedCount.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_queueCount == kQueueSlots) {
                ++m_queueFullCount;
                return true;
            }
            QueuedFrame& slot = m_queue[m_queueTail];
            const size_t rowBytes = static_cast<size_t>(m_inWidth) * 4;
            if (slot.rgba.size() < rowBytes * static_cast<size_t>(m_inHeight)) {
                slot.rgba.assign(rowBytes * m_inHeight, 0);
            }
            if (stride == static_cast<int>(rowBytes)) {
                std::memcpy(slot.rgba.data(), pixels, rowBytes * m_inHeight);
            } else {
                for (int y = 0; y < m_inHeight; ++y) {
                    std::memcpy(slot.rgba.data() + y * rowBytes,
                                pixels + y * stride,
                                rowBytes);
                }
            }
            slot.ptsMicros = relUs;
            slot.valid = true;
            m_queueTail = (m_queueTail + 1) % kQueueSlots;
            ++m_queueCount;
        }
        m_queueCv.notify_one();
        m_lastAcceptedPtsUs = static_cast<uint64_t>(relUs);
        return true;
    }

    bool Stop()
    {
        if (!m_running) return false;

        m_stopRequested.store(true);
        m_queueCv.notify_all();
        if (m_worker.joinable()) {
            m_worker.join();
        }

        // Worker did not flush at EOF — do it here synchronously.
        DbgLog("[Capture] draining encoder at Stop\n");
        DrainEncoder(/*flushing=*/true);

        if (m_fmtCtx) {
            const int rc = av_write_trailer(m_fmtCtx);
            if (rc < 0) LogAvError("av_write_trailer", rc);
        }

        DbgLog("[Capture] recording summary: encoded=%llu dropped=%llu queueFull=%llu encodeErrors=%llu\n",
            static_cast<unsigned long long>(m_encodedCount.load()),
            static_cast<unsigned long long>(m_droppedCount.load()),
            static_cast<unsigned long long>(m_queueFullCount),
            static_cast<unsigned long long>(m_encodeErrorCount));

        DestroyAll();
        m_running = false;
        return true;
    }

    bool IsRunning() const { return m_running; }

    uint64_t EncodedFrameCount() const { return m_encodedCount.load(std::memory_order_relaxed); }
    uint64_t DroppedFrameCount() const { return m_droppedCount.load(std::memory_order_relaxed); }
    int64_t  BitrateKbps() const { return m_bitrateKbps; }
    int      OutputWidth()  const { return m_outWidth; }
    int      OutputHeight() const { return m_outHeight; }

private:
    // Read packets from the encoder until it reports EAGAIN (no more output
    // yet) or EOF (draining complete). When flushing=true, the caller has
    // already sent a null frame to put the encoder into draining mode.
    void DrainEncoder(bool flushing)
    {
        AVPacket* pkt = av_packet_alloc();
        if (!pkt) return;
        while (true) {
            const int ret = avcodec_receive_packet(m_codecCtx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                LogAvError("avcodec_receive_packet", ret);
                ++m_encodeErrorCount;
                break;
            }
            av_packet_rescale_ts(pkt, m_codecCtx->time_base, m_stream->time_base);
            pkt->stream_index = m_stream->index;
            const int wrc = av_interleaved_write_frame(m_fmtCtx, pkt);
            if (wrc < 0) {
                LogAvError("av_interleaved_write_frame", wrc);
                ++m_encodeErrorCount;
                av_packet_unref(pkt);
                break;
            }
            av_packet_unref(pkt);
        }
        av_packet_free(&pkt);
        (void)flushing;
    }

    void WorkerLoop()
    {
        uint64_t frameIndex = 0;
        while (true) {
            QueuedFrame frame;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCv.wait(lock, [this] {
                    return m_queueCount > 0 || m_stopRequested.load();
                });
                if (m_queueCount == 0 && m_stopRequested.load()) {
                    // Signal end-of-stream to the encoder so any buffered
                    // frames flush through the upcoming DrainEncoder call in
                    // Stop(). Sending nullptr puts the encoder into EOF mode.
                    const int sendNull = avcodec_send_frame(m_codecCtx, nullptr);
                    if (sendNull < 0 && sendNull != AVERROR_EOF) {
                        LogAvError("avcodec_send_frame(null)", sendNull);
                    }
                    DrainEncoder(/*flushing=*/true);
                    return;
                }
                QueuedFrame& slot = m_queue[m_queueHead];
                frame = std::move(slot);
                slot.valid = false;
                m_queueHead = (m_queueHead + 1) % kQueueSlots;
                --m_queueCount;
            }

            if (!frame.valid) continue;

            const int mwrc = av_frame_make_writable(m_frame);
            if (mwrc < 0) {
                LogAvError("av_frame_make_writable", mwrc);
                ++m_encodeErrorCount;
                continue;
            }

            const uint8_t* srcSlices[1] = { frame.rgba.data() };
            const int srcStrides[1] = { m_inWidth * 4 };
            const int scaled = sws_scale(m_sws, srcSlices, srcStrides, 0, m_inHeight,
                                         m_frame->data, m_frame->linesize);
            if (scaled <= 0) {
                DbgLog("[Capture] sws_scale produced %d rows (frame %llu)\n",
                    scaled, static_cast<unsigned long long>(frameIndex));
                ++m_encodeErrorCount;
                continue;
            }

            // pts in milliseconds, matching codec time_base = 1/1000.
            m_frame->pts = frame.ptsMicros / 1000;
            const int src = avcodec_send_frame(m_codecCtx, m_frame);
            if (src < 0) {
                LogAvError("avcodec_send_frame", src);
                ++m_encodeErrorCount;
                continue;
            }
            DrainEncoder(/*flushing=*/false);
            const uint64_t encSoFar = m_encodedCount.load(std::memory_order_relaxed);
            if (encSoFar == 0 || (frameIndex % 30) == 0) {
                DbgLog("[Capture] encoded frame %llu\n",
                    static_cast<unsigned long long>(frameIndex));
            }
            m_encodedCount.fetch_add(1, std::memory_order_relaxed);
            ++frameIndex;
        }
    }

    void DestroyAll()
    {
        if (m_sws) {
            sws_freeContext(m_sws);
            m_sws = nullptr;
        }
        if (m_frame) {
            av_frame_free(&m_frame);
        }
        if (m_codecCtx) {
            avcodec_free_context(&m_codecCtx);
        }
        if (m_fmtCtx) {
            if (m_fmtCtx->pb && !(m_fmtCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&m_fmtCtx->pb);
            }
            avformat_free_context(m_fmtCtx);
            m_fmtCtx = nullptr;
        }
        m_stream = nullptr;
    }

    AVFormatContext* m_fmtCtx   = nullptr;
    AVStream*        m_stream   = nullptr;
    AVCodecContext*  m_codecCtx = nullptr;
    AVFrame*         m_frame    = nullptr;
    SwsContext*      m_sws      = nullptr;

    int      m_inWidth  = 0;
    int      m_inHeight = 0;
    int      m_outWidth  = 0;
    int      m_outHeight = 0;
    int      m_targetFps = 30;
    int64_t  m_bitrateKbps = 0;
    PixelFormat m_inFormat = PixelFormat::Rgba8;
    uint64_t m_frameDurationUs = 33'333ULL;

    int64_t  m_firstPtsMicros = -1;
    uint64_t m_lastAcceptedPtsUs = 0;

    std::array<QueuedFrame, kQueueSlots> m_queue{};
    size_t                 m_queueHead = 0;
    size_t                 m_queueTail = 0;
    size_t                 m_queueCount = 0;
    std::mutex             m_queueMutex;
    std::condition_variable m_queueCv;

    std::thread      m_worker;
    std::atomic<bool> m_stopRequested{false};
    bool             m_running = false;

    std::atomic<uint64_t> m_encodedCount{0};
    std::atomic<uint64_t> m_droppedCount{0};
    uint64_t m_queueFullCount   = 0;
    uint64_t m_encodeErrorCount = 0;
};

WebmRecorder::WebmRecorder() : m_impl(std::make_unique<WebmRecorderImpl>()) {}
WebmRecorder::~WebmRecorder() = default;

bool WebmRecorder::Start(const std::filesystem::path& outPath,
                         int inWidth, int inHeight, PixelFormat inFormat,
                         int outWidth, int outHeight,
                         int targetFps, int bitrateKbps)
{
    return m_impl->Start(outPath, inWidth, inHeight, inFormat,
                         outWidth, outHeight, targetFps, bitrateKbps);
}

bool WebmRecorder::PushFrame(const uint8_t* pixels, int stride, int64_t ptsMicros)
{
    return m_impl->PushFrame(pixels, stride, ptsMicros);
}

bool WebmRecorder::Stop()
{
    return m_impl->Stop();
}

bool WebmRecorder::IsRunning() const
{
    return m_impl->IsRunning();
}

uint64_t WebmRecorder::EncodedFrameCount() const
{
    return m_impl ? m_impl->EncodedFrameCount() : 0;
}

uint64_t WebmRecorder::DroppedFrameCount() const
{
    return m_impl ? m_impl->DroppedFrameCount() : 0;
}

int64_t WebmRecorder::BitrateKbps() const
{
    return m_impl ? m_impl->BitrateKbps() : 0;
}

int WebmRecorder::OutputWidth() const
{
    return m_impl ? m_impl->OutputWidth() : 0;
}

int WebmRecorder::OutputHeight() const
{
    return m_impl ? m_impl->OutputHeight() : 0;
}

}  // namespace capture
