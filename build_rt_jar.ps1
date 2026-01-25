# PowerShell script to build rt.jar from stubs directory

# Create build/classes directory if it doesn't exist
New-Item -ItemType Directory -Path "build/classes" -Force | Out-Null

# Find all .java files in stubs directory and write to build/sources.txt
Get-ChildItem -Path "stubs" -Recurse -Filter "*.java" | Resolve-Path -Relative | ForEach-Object { $_ -replace '^\.\\', '' } | Out-File -FilePath "build/sources.txt" -Encoding ASCII

# Compile Java files with javac
& javac -source 1.8 -target 1.8 -d "build/classes" @build/sources.txt

# Check if compilation succeeded
if ($LASTEXITCODE -eq 0) {
    # Create jar file
    & jar cf "stubs/rt.jar" -C "build/classes" .
    Write-Host "stubs/rt.jar created successfully"
} else {
    Write-Host "Compilation failed"
}
