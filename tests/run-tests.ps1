param([string]$SdkIncludePath)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $SdkIncludePath) {
    $SdkIncludePath = Join-Path $projectRoot '../.build/sdk-core/pebble/basalt/include'
}
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Force -Path '.build/sdk-host/src' | Out-Null
    # These stand-ins are for host checks; the Pebble build generates real IDs.
    $ids = Get-ChildItem src -Filter '*.c' | ForEach-Object {
        [regex]::Matches((Get-Content -LiteralPath $_.FullName -Raw), 'RESOURCE_ID_[A-Z0-9_]+').Value
    } | Sort-Object -Unique
    $i = 0
    $ids | ForEach-Object { '#define ' + $_ + ' ' + (++$i) } |
        Set-Content '.build/sdk-host/src/resource_ids.auto.h'
    '#pragma once' | Set-Content '.build/sdk-host/message_keys.auto.h'
    & clang -fuse-ld=lld -std=c99 -Wall -Wextra -Werror -Isrc src/voice_duration.c tests/test_voice_duration.c -o .build/test-duration.exe
    if ($LASTEXITCODE -ne 0) { throw 'Duration compilation failed' }
    & ./.build/test-duration.exe
    if ($LASTEXITCODE -ne 0) { throw 'Duration checks failed' }
    if (-not (Test-Path (Join-Path $SdkIncludePath 'pebble.h'))) { throw 'Supply -SdkIncludePath with the official basalt headers.' }
    $flags = @('-fuse-ld=lld', '-ffunction-sections', '-fdata-sections', '-O1', '-std=c99',
        '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-Wno-unused-function',
        '-Wno-format', '-Wno-return-type', '-Wno-incompatible-library-redeclaration',
        '-D_CRT_SECURE_NO_WARNINGS', '-DPBL_PLATFORM_BASALT', '-DPBL_COLOR', '-DPBL_RECT',
        '-DPBL_MICROPHONE', '-Itests/sdk-host', '-I.build/sdk-host', "-I$SdkIncludePath", '-Isrc')
    & clang @flags src/voice_duration.c tests/test_voice_session.c '-Wl,/OPT:REF' -o .build/test-session.exe
    if ($LASTEXITCODE -ne 0) { throw 'Speech session compilation failed' }
    & ./.build/test-session.exe
    if ($LASTEXITCODE -ne 0) { throw 'Speech session checks failed' }
    & clang @flags src/countdown_timer.c tests/test_voice_creation.c '-Wl,/OPT:REF' -o .build/test-creation.exe
    if ($LASTEXITCODE -ne 0) { throw 'Timer creation compilation failed' }
    & ./.build/test-creation.exe
    if ($LASTEXITCODE -ne 0) { throw 'Timer creation checks failed' }
} finally { Pop-Location }
