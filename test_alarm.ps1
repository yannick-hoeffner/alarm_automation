Add-Type -AssemblyName System.Windows.Forms, System.Drawing

$form = New-Object System.Windows.Forms.Form
$form.FormBorderStyle = 'None'
$form.WindowState = 'Maximized'
$form.TopMost = $true
$form.BackColor = [System.Drawing.Color]::FromArgb(200,0,0)
$form.AllowTransparency = $true
$form.Opacity = 0.8 

$label = New-Object System.Windows.Forms.Label
$label.Text = "Feuerwehreinsatz!"
$label.ForeColor = [System.Drawing.Color]::White
$label.Font = New-Object System.Drawing.Font("Arial", 72,[System.Drawing.FontStyle]::Bold)
$label.Dock = 'Fill'
$label.TextAlign = 'MiddleCenter'
$form.Controls.Add($label)

$alarm.Add_KeyDown({ $alarm.Close(); $bg.Close() })

$form.ShowDialog()