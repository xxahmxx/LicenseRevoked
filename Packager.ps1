param(
    [string]$DllPath,
    [string]$IniPath,
    [string]$ReadmePath,
    [string]$ChangelogPath,
    [string]$SolutionDir
)

Write-Host "--- AUTO-PACKAGER START ---" -ForegroundColor Cyan

# 1. Sprawdzenie DLL
if (-not (Test-Path $DllPath)) { Write-Error "Brak pliku DLL!"; exit 1 }

# 2. Pobranie wersji
try { $ver = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($DllPath).FileVersion } catch { $ver = "1.0.0.0" }
if (-not $ver) { $ver = "1.0.0.0" }
Write-Host "Wykryta wersja: $ver" -ForegroundColor Yellow

# 3. Ścieżki
$releasesDir = Join-Path $SolutionDir "Releases"
$tempDir = Join-Path $releasesDir "TempPackage"
$zipName = "LicenseRevoked_v$ver.zip"
$zipPath = Join-Path $releasesDir $zipName
$pluginDest = Join-Path $tempDir "bin\win_x64\plugins"

# 4. Czyszczenie
if (Test-Path $tempDir) { Remove-Item $tempDir -Recurse -Force }
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }

# 5. Foldery
New-Item -ItemType Directory -Path $pluginDest -Force | Out-Null

# 6. Kopia archiwalna DLL
$originalName = [System.IO.Path]::GetFileNameWithoutExtension($DllPath)
Copy-Item $DllPath (Join-Path $releasesDir "$originalName`_v$ver.dll") -Force

# 7. Kopiowanie plików do paczki
Copy-Item $DllPath $pluginDest
Copy-Item $IniPath $pluginDest

if (Test-Path $ReadmePath) { Copy-Item $ReadmePath $tempDir }

# --- CZYSZCZENIE CHANGELOG (Safe Encoding Version) ---
if (Test-Path $ChangelogPath) {
    Write-Host "Konwertowanie CHANGELOG do czystego tekstu..." -ForegroundColor Gray
    $content = Get-Content $ChangelogPath -Raw
    
    # CZYSZCZENIE MARKDOWN
    $clean = $content
    $clean = $clean -replace '(?m)^#+\s*', ''          
    $clean = $clean -replace '\*\*([^\*]+)\*\*', '$1'  
    $clean = $clean -replace '__([^__]+)__', '$1'      
    $clean = $clean -replace '\*([^\*]+)\*', '$1'      
    $clean = $clean -replace '_([^_]+)_', '$1'        
    $clean = $clean -replace '\[([^\]]+)\]\([^\)]+\)', '$1' 
    $clean = $clean -replace '`([^`]+)`', '$1'
    # Używamy zwykłego myślnika zamiast kropki specjalnej
    $clean = $clean -replace '(?m)^\s*-\s*', '- '      

    $changelogDest = Join-Path $tempDir "CHANGELOG.txt"
    
    # WYMUSZENIE KODOWANIA UTF8 BEZ BOM (Najbardziej kompatybilne)
    [System.IO.File]::WriteAllLines($changelogDest, $clean)
    
    Write-Host "Dodano wyczyszczony CHANGELOG.txt." -ForegroundColor Gray
}

# 8. Pakowanie
Write-Host "Tworzenie ZIP..." -ForegroundColor Gray
Compress-Archive -Path "$tempDir\*" -DestinationPath $zipPath -Force

# 9. Sprzątanie
Remove-Item $tempDir -Recurse -Force

Write-Host "--- SUKCES: Utworzono $zipName ---" -ForegroundColor Green