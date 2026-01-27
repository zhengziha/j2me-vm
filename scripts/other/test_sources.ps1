# Test script to diagnose sources.txt generation issue

Write-Host "Current directory: $PWD"

# List all .java files
Write-Host "\nListing all .java files:"
$javaFiles = Get-ChildItem -Path "stubs" -Recurse -Filter "*.java"
Write-Host "Found $($javaFiles.Count) .java files"

# Display the first few files
Write-Host "\nFirst 10 files:"
$javaFiles | Select-Object -First 10 | ForEach-Object {
    Write-Host "  $($_.FullName)"
}

# Try to generate sources.txt
Write-Host "\nGenerating sources.txt:"
$javaFiles | Resolve-Path -Relative | ForEach-Object { $_ -replace '^\.\\', '' } | Out-File -FilePath "build\test_sources.txt" -Encoding ASCII

# Check the content of test_sources.txt
Write-Host "\nContent of test_sources.txt:"
Get-Content "build\test_sources.txt"

# Count the lines
Write-Host "\nLine count:"
(Get-Content "build\test_sources.txt" | Measure-Object -Line).Lines
