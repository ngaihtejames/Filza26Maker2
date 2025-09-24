#!/bin/bash
# WSL / Ubuntu-ready Filza DEB → IPA builder
# Works for IOS 18 / IOS 26 jailed Filza

set -eo pipefail

# CONFIG
DEB_URL="${1:-https://tigisoftware.com/cydia/com.tigisoftware.filza_4.0.1-2_iphoneos-arm.deb}"
WIN_DESKTOP="/mnt/c/Users/$(cmd.exe /c "echo %USERNAME%" | tr -d '\r')/Desktop"
WORKDIR="$(mktemp -d)"
DEB_LOCAL="$WORKDIR/filza.deb"
IPA_NAME="Filza-Jailed-26-UncleTyrone.ipa"

echo "[ i ] Working dir: $WORKDIR"
echo "[ i ] Target DEB URL: $DEB_URL"
echo

# Download the DEB (or use local if provided)
if [[ -f "$1" && "$1" != "$DEB_URL" ]]; then
  echo "[ i ] Using local file: $1"
  cp "$1" "$DEB_LOCAL"
else
  echo "[ i ] Downloading DEB..."
  curl -L --fail -o "$DEB_LOCAL" "$DEB_URL"
fi

# ensure ar exists
if ! command -v ar >/dev/null 2>&1; then
  echo "[!] 'ar' not found. Install binutils: sudo apt install binutils"
  exit 1
fi

cd "$WORKDIR"

echo "[ i ] Extracting .deb with ar..."
ar -x "$DEB_LOCAL"

# find tarball (data.tar.*)
DATA_TAR="$(ls data.tar.* 2>/dev/null | head -n1 || true)"
if [[ -z "$DATA_TAR" ]]; then
  echo "[-] data.tar.* not found in deb. Listing content:"
  ar -t "$DEB_LOCAL"
  exit 1
fi

echo "[ i ] Found $DATA_TAR - extracting..."
mkdir data_extracted
tar -xf "$DATA_TAR" -C data_extracted

# locate Filza.app inside extracted payloads
echo "[ i ] Searching for Filza.app..."
FILZA_APP_PATH="$(find data_extracted -type d -iname 'Filza.app' | head -n1 || true)"

if [[ -z "$FILZA_APP_PATH" ]]; then
  echo "[-] Filza.app not found. Here's a quick list of top-level dirs:"
  find data_extracted -maxdepth 3 -type d -print | sed -n '1,50p'
  exit 1
fi

echo "[ + ] Found Filza.app at: $FILZA_APP_PATH"
echo "[ i ] Preparing Payload directory..."
mkdir -p Payload
cp -R "$FILZA_APP_PATH" Payload/

# optional: remove any code signature / provisioning info
rm -rf Payload/Filza.app/_CodeSignature 2>/dev/null || true
rm -f Payload/Filza.app/embedded.mobileprovision 2>/dev/null || true

echo "[ i ] Creating IPA (zip)..."
zip -r "$IPA_NAME" Payload > /dev/null

# move IPA to Windows Desktop
echo "[ i ] Moving IPA to Windows Desktop: $WIN_DESKTOP/$IPA_NAME"
cp -f "$IPA_NAME" "$WIN_DESKTOP/" && rm -f "$IPA_NAME"

# cleanup
echo "[ i ] Cleaning up temporary files..."
rm -rf "$WORKDIR"

echo
echo "[ + ] Done. IPA placed at: $WIN_DESKTOP/$IPA_NAME"
echo "[ ! ] Unsigned. Use Sideloadly / AltStore / ESign to sign & install."