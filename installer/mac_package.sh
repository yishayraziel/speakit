#!/bin/bash
# Signs (Developer ID), notarizes and packages Anal-Log for macOS.
# Without the signing secrets it falls back to an ad-hoc signed, un-notarized build.
set -euo pipefail
# surface failures as GitHub annotations (readable without the raw logs)
trap 'echo "::error title=mac_package.sh line $LINENO::failed: $BASH_COMMAND"' ERR
staple() {     # Apple's ticket can take a minute to propagate after "Accepted": retry
  for i in 1 2 3 4 5 6 7 8; do
    xcrun stapler staple "$1" && return 0
    echo "stapler not ready yet ($i), retrying in 20 s"; sleep 20
  done
  echo "::error title=stapler::could not staple $1"; return 1
}
notarize() {   # notarize "$1" and fail with Apple's log if it isn't accepted
  local out id status
  out=$(xcrun notarytool submit "$1" "${NOTARY[@]}" --output-format json 2>&1) || { echo "::error title=notarytool::$(echo "$out" | tr '\n' ' ' | cut -c1-900)"; return 1; }
  id=$(echo "$out" | sed -nE 's/.*"id" *: *"([^"]+)".*/\1/p' | head -1)
  status=$(echo "$out" | sed -nE 's/.*"status" *: *"([^"]+)".*/\1/p' | head -1)
  echo "::notice title=notarize $(basename "$1")::status=$status id=$id"
  if [ "$status" != "Accepted" ]; then
    echo "::error title=notary log::$(xcrun notarytool log "$id" "${NOTARY_AUTH[@]}" 2>&1 | tr '\n' ' ' | cut -c1-1500)"
    return 1
  fi
}
VERSION="${VERSION:-1.0.0}"
APP="build/SPEAKIT_artefacts/Release/Standalone/Anal-Log.app"
AU="build/SPEAKIT_artefacts/Release/AU/Anal-Log.component"
VST="build/SPEAKIT_artefacts/Release/VST3/Anal-Log.vst3"
OUT="$PWD/out"; mkdir -p "$OUT" pkgroot/vst3 pkgroot/au pkgroot/app dmg

SIGNED=0
if [ -n "${MACOS_CERT_APP_P12:-}" ] && [ -n "${MACOS_CERT_INSTALLER_P12:-}" ]; then
  SIGNED=1
  KC="$RUNNER_TEMP/speakit.keychain-db"; KCP="$(uuidgen)"
  security create-keychain -p "$KCP" "$KC"
  security set-keychain-settings -lut 21600 "$KC"
  security unlock-keychain -p "$KCP" "$KC"
  echo "$MACOS_CERT_APP_P12"       | base64 --decode > "$RUNNER_TEMP/app.p12"
  echo "$MACOS_CERT_INSTALLER_P12" | base64 --decode > "$RUNNER_TEMP/inst.p12"
  for p in app inst; do
    imp=$(security import "$RUNNER_TEMP/$p.p12" -k "$KC" -P "$MACOS_CERT_PASSWORD" -T /usr/bin/codesign -T /usr/bin/productbuild -T /usr/bin/productsign 2>&1) \
      || { echo "::error title=import $p.p12::$imp"; exit 1; }
  done
  security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$KCP" "$KC" >/dev/null
  security list-keychains -d user -s "$KC" $(security list-keychains -d user | tr -d '"')
  APP_ID=$(security find-identity -v -p codesigning "$KC" | grep "Developer ID Application" | head -1 | sed -E 's/.*"(.*)"/\1/')
  INST_ID=$(security find-identity -v "$KC" | grep "Developer ID Installer" | head -1 | sed -E 's/.*"(.*)"/\1/')
  echo "::notice title=identities::app='${APP_ID:-NONE}' installer='${INST_ID:-NONE}' ($(security find-identity -v "$KC" | tail -1))"
  [ -n "$APP_ID" ] && [ -n "$INST_ID" ] || { echo "::error title=certificates::Developer ID Application/Installer identity not found in the imported p12 files"; exit 1; }
  NOTARY_AUTH=(--apple-id "$APPLE_ID" --password "$APPLE_APP_PASSWORD" --team-id "$APPLE_TEAM_ID")
  NOTARY=("${NOTARY_AUTH[@]}" --wait)

  codesign --force --options runtime --timestamp --sign "$APP_ID" "$VST"
  codesign --force --options runtime --timestamp --sign "$APP_ID" "$AU"
  codesign --force --options runtime --timestamp --entitlements installer/entitlements.plist --sign "$APP_ID" "$APP"
  codesign --verify --strict --verbose=2 "$VST" "$AU" "$APP"

  # notarize the bare VST3 (for the manual-install zip) and staple the ticket to the bundle
  ditto -c -k --keepParent "$VST" "$RUNNER_TEMP/vst3.zip"
  notarize "$RUNNER_TEMP/vst3.zip"
  staple "$VST"
else
  echo "No signing secrets: ad-hoc signing (users will see a Gatekeeper warning)"
  codesign --force --deep -s - "$VST"
  codesign --force --deep -s - "$AU"
  codesign --force --deep -s - "$APP"
fi

(cd "$(dirname "$VST")" && ditto -c -k --keepParent Anal-Log.vst3 "$OUT/Anal-Log_mac_VST3.zip")

cp -R "$VST" pkgroot/vst3/
cp -R "$AU" pkgroot/au/
cp -R "$APP" pkgroot/app/
pkgbuild --root pkgroot/vst3 --identifier com.yraziel.anallog.vst3 --version "$VERSION" --install-location "/Library/Audio/Plug-Ins/VST3" vst3.pkg
pkgbuild --root pkgroot/au --identifier com.yraziel.anallog.au --version "$VERSION" --install-location "/Library/Audio/Plug-Ins/Components" au.pkg
pkgbuild --root pkgroot/app  --identifier com.yraziel.anallog.app  --version "$VERSION" --install-location "/Applications" app.pkg
if [ $SIGNED = 1 ]; then
  productbuild --sign "$INST_ID" --timestamp --package vst3.pkg --package au.pkg --package app.pkg "dmg/Anal-Log Installer.pkg"
  notarize "dmg/Anal-Log Installer.pkg"
  staple "dmg/Anal-Log Installer.pkg"
else
  productbuild --package vst3.pkg --package au.pkg --package app.pkg "dmg/Anal-Log Installer.pkg"
fi
cp docs/Anal-Log_Manual.pdf dmg/ || true
DMG_MB=$(( $(du -sm dmg | cut -f1) * 13 / 10 + 40 ))
hdiutil create -volname "Anal-Log" -srcfolder dmg -fs HFS+ -size "${DMG_MB}m" -ov -format UDZO "$OUT/Anal-Log_mac.dmg"
if [ $SIGNED = 1 ]; then
  codesign --force --timestamp --sign "$APP_ID" "$OUT/Anal-Log_mac.dmg"
  notarize "$OUT/Anal-Log_mac.dmg"
  staple "$OUT/Anal-Log_mac.dmg"
  spctl -a -vvv -t install "dmg/Anal-Log Installer.pkg" || true
fi
ls -la "$OUT"
