param(
    [string]$RcPath
)

if (-not (Test-Path $RcPath)) {
    Write-Host "Nie znaleziono pliku Resource (.rc)" -ForegroundColor Red
    exit 0
}

$content = Get-Content $RcPath -Raw

# Regexy do znajdowania wersji w formacie X,X,X,X (Struct) oraz "X.X.X.X" (String)
# Szukamy ostatniej cyfry (Build Number)

# 1. Zmieniamy FILEVERSION 1,2,0,5 -> 1,2,0,6
$content = [Regex]::Replace($content, '(FILEVERSION\s+\d+,\d+,\d+,)(\d+)', {
    param($match)
    $prefix = $match.Groups[1].Value
    $number = [int]$match.Groups[2].Value + 1
    return "$prefix$number"
})

# 2. Zmieniamy PRODUCTVERSION 1,2,0,5 -> 1,2,0,6
$content = [Regex]::Replace($content, '(PRODUCTVERSION\s+\d+,\d+,\d+,)(\d+)', {
    param($match)
    $prefix = $match.Groups[1].Value
    $number = [int]$match.Groups[2].Value + 1
    return "$prefix$number"
})

# 3. Zmieniamy "FileVersion", "1.2.0.5" -> "1.2.0.6"
$content = [Regex]::Replace($content, '("FileVersion",\s*"\d+\.\d+\.\d+\.)(\d+)"', {
    param($match)
    $prefix = $match.Groups[1].Value
    $number = [int]$match.Groups[2].Value + 1
    return "$prefix$number`""
})

# 4. Zmieniamy "ProductVersion", "1.2.0.5" -> "1.2.0.6"
$content = [Regex]::Replace($content, '("ProductVersion",\s*"\d+\.\d+\.\d+\.)(\d+)"', {
    param($match)
    $prefix = $match.Groups[1].Value
    $number = [int]$match.Groups[2].Value + 1
    return "$prefix$number`""
})

Set-Content $RcPath $content -NoNewline
Write-Host "Zwiekszono wersje w pliku .rc" -ForegroundColor Cyan