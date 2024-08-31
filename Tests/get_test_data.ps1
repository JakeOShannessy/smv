$commit_fig = "374d9db0062fd6dc38b30bc0d27cd84c9bb2b69d"

$zip_path = "$env:temp\test-data.zip"
$unzip_path = "$env:temp\test-data-out"
Invoke-WebRequest -Uri https://github.com/firemodels/fig/archive/$commit_fig.zip -OutFile $zip_path
Expand-Archive $zip_path -DestinationPath $unzip_path -Force
mkdir fig -Force
Move-Item $unzip_path/fig-$commit_fig/* fig -Force
Copy-Item -Force -Recurse fig/smv/Tests/Visualization ../Verification
