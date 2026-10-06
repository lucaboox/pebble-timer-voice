$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$destination = Join-Path (Split-Path -Parent $projectRoot) 'Pebble-Timer-Voice-CloudPebble.zip'
$files = @('package.json', 'wscript', 'README.md', 'VOICE-README.md', '.gitignore')
foreach ($directory in @('src', 'resources', 'tests', 'tools')) {
    $files += Get-ChildItem (Join-Path $projectRoot $directory) -Recurse -File |
        ForEach-Object { $_.FullName.Substring($projectRoot.Length + 1).Replace('\', '/') }
}
Add-Type -AssemblyName System.IO.Compression
$stream = [System.IO.File]::Open($destination, [System.IO.FileMode]::Create)
$archive = [System.IO.Compression.ZipArchive]::new($stream, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($relative in $files) {
        $entry = $archive.CreateEntry($relative, [System.IO.Compression.CompressionLevel]::Optimal)
        $inputStream = [System.IO.File]::OpenRead((Join-Path $projectRoot $relative))
        $outputStream = $entry.Open()
        try { $inputStream.CopyTo($outputStream) }
        finally { $outputStream.Dispose(); $inputStream.Dispose() }
    }
} finally { $archive.Dispose(); $stream.Dispose() }
Write-Output $destination
