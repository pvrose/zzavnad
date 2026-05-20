# clear_icon_cache.ps1
# Clear Windows icon cache without rebooting
# Run as Administrator for best results

Write-Host "Clearing Windows icon cache..." -ForegroundColor Cyan

# Stop Windows Explorer
Write-Host "Stopping Windows Explorer..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force

# Wait for Explorer to stop
Start-Sleep -Seconds 2

# Clear icon cache files
$iconCachePaths = @(
	"$env:LOCALAPPDATA\IconCache.db",
	"$env:LOCALAPPDATA\Microsoft\Windows\Explorer\iconcache_*.db"
)

foreach ($path in $iconCachePaths) {
	$files = Get-Item $path -ErrorAction SilentlyContinue
	foreach ($file in $files) {
		try {
			Remove-Item $file.FullName -Force -ErrorAction Stop
			Write-Host "Deleted: $($file.FullName)" -ForegroundColor Green
		}
		catch {
			Write-Host "Could not delete: $($file.FullName) - $($_.Exception.Message)" -ForegroundColor Red
		}
	}
}

# Also clear thumbnail cache
$thumbCachePath = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db"
$thumbFiles = Get-Item $thumbCachePath -ErrorAction SilentlyContinue
foreach ($file in $thumbFiles) {
	try {
		Remove-Item $file.FullName -Force -ErrorAction Stop
		Write-Host "Deleted thumbnail cache: $($file.FullName)" -ForegroundColor Green
	}
	catch {
		Write-Host "Could not delete thumbnail cache: $($file.FullName)" -ForegroundColor Red
	}
}

# Restart Windows Explorer
Write-Host "Restarting Windows Explorer..." -ForegroundColor Yellow
Start-Process explorer.exe

Write-Host "`nIcon cache cleared! Your taskbar icons should refresh shortly." -ForegroundColor Green
Write-Host "If icons don't update immediately, try unpinning and re-pinning the application." -ForegroundColor Cyan
