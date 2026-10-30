param(
    [Parameter(Mandatory=$true)][string]$SdkRoot,
    [string]$Image = 'xiashj/logue-sdk@sha256:e4d85a16c38dc4d34b0e93cabb6378df21729688dbcd88808c84e8d41aa0c9ba'
)
$ErrorActionPreference = 'Stop'
$sdkPath = (Resolve-Path -LiteralPath $SdkRoot).Path
$platformPath = Join-Path $sdkPath 'platform'
$commonPath = Join-Path $platformPath 'drumlogue/common/unit.h'
if (!(Test-Path -LiteralPath $commonPath)) { throw 'SdkRoot must point to a logue-sdk checkout.' }
$projectPath = Join-Path $platformPath 'drumlogue/age-carrier-v040'
if ((Test-Path -LiteralPath $projectPath) -and ((Resolve-Path -LiteralPath $projectPath).Path -ne $PSScriptRoot)) {
    throw "Target already exists: $projectPath. Build that checkout directly or choose a fresh SDK checkout."
}
docker info --format '{{.ServerVersion}}'
if ($LASTEXITCODE -ne 0) { throw 'Docker engine is unavailable.' }
if (!(Test-Path -LiteralPath $projectPath)) {
    New-Item -ItemType Directory -Path $projectPath | Out-Null
    foreach ($file in @('Makefile','config.mk','header.c','unit.cc','synth.h')) {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot $file) -Destination $projectPath
    }
}
docker run --rm --entrypoint /bin/bash --mount "type=bind,source=$platformPath,target=/workspace" $Image -c 'set -e; source /app/env; env drumlogue; cd /workspace/drumlogue/age-carrier-v040; make SKIP_CHOWN=yes; make install'
if ($LASTEXITCODE -ne 0) { throw 'ARM build failed.' }
$binaryPath = Join-Path $projectPath 'age_carrier.drmlgunit'
if (!(Test-Path -LiteralPath $binaryPath)) { throw 'Build returned no unit file.' }
if ($projectPath -ne $PSScriptRoot) {
    Copy-Item -LiteralPath $binaryPath -Destination (Join-Path $PSScriptRoot 'age_carrier.drmlgunit')
}
Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'age_carrier.drmlgunit') -Algorithm SHA256
