param([Parameter(Mandatory=$true)][string]$SdkRoot)
$ErrorActionPreference = 'Stop'
$includeCommon = Join-Path $SdkRoot 'platform/drumlogue/common'
$includeExt = Join-Path $SdkRoot 'platform/ext'
if (!(Test-Path (Join-Path $includeCommon 'unit.h'))) { throw 'Invalid logue-sdk root' }
if (!(Get-Command cl -ErrorAction SilentlyContinue)) { throw 'Run from a Visual Studio developer PowerShell.' }
$testBuild = Join-Path $PSScriptRoot 'build-host'
New-Item -ItemType Directory -Force $testBuild | Out-Null
Push-Location $testBuild
try {
  '#ifdef _MSC_VER', '#define __attribute__(...)', '#endif' | Set-Content msvc_compat.h
  $flags = @('/nologo','/EHsc','/std:c++17','/O2','/I.','/FI','msvc_compat.h',"/I$includeCommon","/I$includeExt")
  & cl @flags (Join-Path $PSScriptRoot 'tests/dsp_test.cpp') '/Fe:dsp_test.exe'
  if ($LASTEXITCODE) { throw 'DSP compile failed' }
  & ./dsp_test.exe
  if ($LASTEXITCODE) { throw 'DSP test failed' }
  & cl @flags (Join-Path $PSScriptRoot 'tests/extended_test.cpp') '/Fe:extended_test.exe'
  if ($LASTEXITCODE) { throw 'Extended chord compile failed' }
  & ./extended_test.exe
  if ($LASTEXITCODE) { throw 'Extended chord test failed' }
  & cl @flags /c (Join-Path $PSScriptRoot 'unit.cc') (Join-Path $PSScriptRoot 'tests/sdk_test.cpp')
  if ($LASTEXITCODE) { throw 'SDK compile failed' }
  & cl @flags /c (Join-Path $PSScriptRoot 'header.c')
  if ($LASTEXITCODE) { throw 'Header compile failed' }
  & link /nologo unit.obj sdk_test.obj header.obj /out:sdk_test.exe
  if ($LASTEXITCODE) { throw 'Link failed' }
  & ./sdk_test.exe
  if ($LASTEXITCODE) { throw 'SDK test failed' }
} finally { Pop-Location }
