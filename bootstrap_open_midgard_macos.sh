#!/usr/bin/env bash
#
# macOS setup + build script for open-midgard.
#
# On an interactive terminal the script prompts for each setting with a sane
# default (and an auto-detect hint where possible). For unattended runs, set
# any of these as environment variables or pass --yes to accept all defaults:
#   REPO_URL      Git remote to clone (default: upstream)
#   BRANCH        Branch to build (default: dev)
#   PROJECT_DIR   Checkout directory name (default: open-midgard)
#   BUILD_DIR     CMake build dir (default: build-macos)
#   VULKAN_SDK    Path to the Vulkan SDK macOS prefix. If unset, auto-detects
#                 the newest $HOME/VulkanSDK/*/macOS install.
#   OLDRO_DIR     Destination for the deployed .app (default: $HOME/Downloads/OldRO)

set -euo pipefail

ASSUME_YES=0
for arg in "$@"; do
  case "$arg" in
    -y|--yes) ASSUME_YES=1 ;;
    -h|--help)
      sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'
      exit 0
      ;;
  esac
done

detect_vulkan_sdk() {
  if [ -n "${VULKAN_SDK:-}" ] && [ -f "$VULKAN_SDK/lib/libvulkan.dylib" ]; then
    echo "$VULKAN_SDK"
    return 0
  fi
  local candidate
  candidate=$(ls -d "$HOME"/VulkanSDK/*/macOS 2>/dev/null | sort -V | tail -n 1 || true)
  if [ -n "$candidate" ] && [ -f "$candidate/lib/libvulkan.dylib" ]; then
    echo "$candidate"
    return 0
  fi
  return 1
}

# prompt_var <var-name> <default> [hint]
# - If the var is already set in the environment, keep it (no prompt).
# - If stdin is not a TTY or --yes was passed, use the default silently.
# - Otherwise prompt; Enter accepts the default.
prompt_var() {
  local name="$1" default="$2" hint="${3:-}"
  local current="${!name:-}"
  if [ -n "$current" ]; then
    echo "  $name = $current  (from environment)"
    return 0
  fi
  if [ "$ASSUME_YES" -eq 1 ] || [ ! -t 0 ]; then
    printf -v "$name" '%s' "$default"
    export "$name"
    echo "  $name = $default"
    return 0
  fi
  local label="$name"
  if [ -n "$hint" ]; then
    label="$label ($hint)"
  fi
  local answer
  read -r -p "  $label [$default]: " answer || answer=""
  if [ -z "$answer" ]; then
    answer="$default"
  fi
  printf -v "$name" '%s' "$answer"
  export "$name"
}

echo "==> Configuring settings (press Enter to accept defaults)..."

DEFAULT_VULKAN=""
VULKAN_HINT=""
if DEFAULT_VULKAN="$(detect_vulkan_sdk)"; then
  VULKAN_HINT="auto-detected"
else
  DEFAULT_VULKAN="$HOME/VulkanSDK/<version>/macOS"
  VULKAN_HINT="not found — install from https://vulkan.lunarg.com/sdk/home#mac"
fi

prompt_var REPO_URL    "https://github.com/SalmanTKhan/open-midgard.git"
prompt_var BRANCH      "dev"
prompt_var PROJECT_DIR "open-midgard"
prompt_var BUILD_DIR   "build-macos"
prompt_var VULKAN_SDK  "$DEFAULT_VULKAN" "$VULKAN_HINT"
prompt_var OLDRO_DIR   "$HOME/Downloads/OldRO"

if [ ! -f "$VULKAN_SDK/lib/libvulkan.dylib" ]; then
  cat >&2 <<EOF
ERROR: Vulkan SDK not found at: $VULKAN_SDK
Install from https://vulkan.lunarg.com/sdk/home#mac and re-run, or point
VULKAN_SDK at an existing install.
EOF
  exit 1
fi

echo "==> Checking Homebrew..."
if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required: https://brew.sh/"
  exit 1
fi

echo "==> Installing dependencies..."
brew update
brew install cmake ninja qt@6 git ffmpeg pkg-config

QT_PREFIX="$(brew --prefix qt@6)"
FFMPEG_PREFIX="$(brew --prefix ffmpeg)"
BREW_PREFIX="$(brew --prefix)"
VULKAN_PREFIX="$VULKAN_SDK"

echo "Qt path:     $QT_PREFIX"
echo "FFmpeg path: $FFMPEG_PREFIX"
echo "Vulkan path: $VULKAN_PREFIX"
echo "Deploy dir:  $OLDRO_DIR"

echo "==> Cloning/updating repo (branch: $BRANCH)..."
if [ -d "$PROJECT_DIR" ]; then
  cd "$PROJECT_DIR"
  git fetch origin
  git checkout "$BRANCH"
  git pull origin "$BRANCH"
else
  git clone -b "$BRANCH" "$REPO_URL" "$PROJECT_DIR"
  cd "$PROJECT_DIR"
fi

export PATH="$QT_PREFIX/bin:$PATH"
export CMAKE_PREFIX_PATH="$QT_PREFIX:$VULKAN_PREFIX:$FFMPEG_PREFIX:$BREW_PREFIX"
export PKG_CONFIG_PATH="$FFMPEG_PREFIX/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

echo "==> Cleaning build..."
rm -rf "$BUILD_DIR"

echo "==> Configuring..."
cmake -S . -B "$BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DRO_ENABLE_DEV_DEPLOY=OFF \
  -DCMAKE_PREFIX_PATH="$QT_PREFIX:$VULKAN_PREFIX:$FFMPEG_PREFIX:$BREW_PREFIX" \
  -DVulkan_INCLUDE_DIR="$VULKAN_PREFIX/include" \
  -DVulkan_LIBRARY="$VULKAN_PREFIX/lib/libvulkan.dylib" \
  -DFFMPEG_DIR="$FFMPEG_PREFIX"

echo "==> Building..."
cmake --build "$BUILD_DIR" -j"$(sysctl -n hw.ncpu)"

echo "==> Locating executable..."
BIN_PATH="$(find "$BUILD_DIR" -type f -perm +111 -name "open-midgard" | head -n 1)"
if [ -z "$BIN_PATH" ]; then
  echo "ERROR: open-midgard binary not found"
  exit 1
fi
echo "Binary found at: $BIN_PATH"

echo "==> Creating .app bundle..."
APP_DIR="$BUILD_DIR/open-midgard.app"
mkdir -p "$APP_DIR/Contents/MacOS"
cp "$BIN_PATH" "$APP_DIR/Contents/MacOS/"

echo "==> Running macdeployqt..."
macdeployqt "$APP_DIR"

echo "==> Copying app to $OLDRO_DIR..."
mkdir -p "$OLDRO_DIR"
rm -rf "$OLDRO_DIR/open-midgard.app"
cp -R "$APP_DIR" "$OLDRO_DIR/"

echo "==> Creating launch script..."
LAUNCH_SCRIPT="$OLDRO_DIR/run_open_midgard.sh"
cat > "$LAUNCH_SCRIPT" <<EOF
#!/usr/bin/env bash
set -e

VULKAN_ROOT="\${VULKAN_SDK:-$VULKAN_PREFIX}"

export DYLD_LIBRARY_PATH="\$VULKAN_ROOT/lib:\${DYLD_LIBRARY_PATH:-}"
export VK_ICD_FILENAMES="\$VULKAN_ROOT/share/vulkan/icd.d/MoltenVK_icd.json"

cd "\$(dirname "\$0")"
exec "./open-midgard.app/Contents/MacOS/open-midgard" "\$@"
EOF
chmod +x "$LAUNCH_SCRIPT"

echo "==> Signing app bundle..."
codesign --force --deep --sign - "$OLDRO_DIR/open-midgard.app"

echo "==> Done."
echo ""
echo "Run with:"
echo "  $LAUNCH_SCRIPT"
