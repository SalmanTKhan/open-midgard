# Qt Notice Bundle

This directory contains the files that the build copies into deployed outputs when `RO_ENABLE_QT6_UI_SPIKE=ON`.

## Contents

- `NOTICE.txt`
- `INSTALLATION_INFORMATION.txt`
- `SOURCE_OFFER.txt`
- `COPYING.LGPL-3.0.txt`
- `COPYING.GPL-3.0.txt`

## Why These Files Are Here

Qt's open-source guidance for LGPL distribution calls out a few things that should be visible in a shipped build:

- prominent notice that Qt is used under LGPL;
- copies of the LGPL and GPL license texts;
- the user's ability to replace and re-link the LGPL library when dynamic linking is used;
- access to the complete corresponding source code for the shipped Qt libraries, including modifications, or an equivalent written offer.

This directory gives the project a repeatable place for that packaging material so `windeployqt` deployment is not the only thing we automate.

## Release Checklist

Before shipping a Qt-enabled build, make sure the release process also does all of the following:

1. Record the exact Qt version and kit that was used to build the release.
2. Archive or otherwise make available the exact corresponding Qt source for that version, including any modifications.
3. Ship this notice bundle next to the deployed executable and Qt DLLs.
4. Avoid distribution terms or platform restrictions that would block LGPL replacement / debugging rights.

This repository provides the technical hooks for those steps, but it does not replace legal review.
