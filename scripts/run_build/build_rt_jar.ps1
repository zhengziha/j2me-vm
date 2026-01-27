# PowerShell script to build rt.jar from stubs directory

# Create build/classes directory if it doesn't exist
New-Item -ItemType Directory -Path "build/classes" -Force | Out-Null

# Generate sources.txt with all .java files
Write-Host "Generating sources.txt..."
$javaFiles = Get-ChildItem -Path "stubs" -Recurse -Filter "*.java"

# Clear existing sources.txt
Clear-Content -Path "build/sources.txt" -ErrorAction SilentlyContinue

# Add each Java file path to sources.txt
foreach ($file in $javaFiles) {
    $relativePath = $file.FullName -replace [regex]::Escape($PWD.Path), ''
    $relativePath = $relativePath -replace '^\\', ''
    Add-Content -Path "build/sources.txt" -Value $relativePath
}

Write-Host "Found $($javaFiles.Count) Java files"

# Compile Java files with javac
Write-Host "Compiling Java files..."
& javac -source 1.8 -target 1.8 -d "build/classes" @build/sources.txt

# Check if compilation succeeded
if ($LASTEXITCODE -eq 0) {
    # Create jar file
    Write-Host "Creating rt.jar..."
    & jar cf "stubs/rt.jar" -C "build/classes" .
    Write-Host "stubs/rt.jar created successfully"
} else {
    Write-Host "Compilation failed"
}
