Add-Type -AssemblyName System.Windows.Forms, System.Drawing

# Fullscreen translucent red background
$bg = New-Object System.Windows.Forms.Form
$bg.FormBorderStyle = 'None'
$bg.WindowState = 'Maximized'
$bg.TopMost = $true
$bg.AllowTransparency = $true
$bg.Opacity = 0.6            # background translucency (0.0 - 1.0)
$bg.BackColor = [System.Drawing.Color]::FromArgb(200,0,0)

# Opaque alarm window containing the label
$alarm = New-Object System.Windows.Forms.Form
$alarm.FormBorderStyle = 'None'
$alarm.StartPosition = 'Manual'
$alarm.TopMost = $true
$alarm.BackColor = [System.Drawing.Color]::Red
$alarm.Opacity = 1.0         # keep this window fully opaque
$alarm.Size = New-Object System.Drawing.Size(1000,400)

# Center alarm on primary screen
$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$alarm.Location = New-Object System.Drawing.Point(
    [int](($screen.Width - $alarm.Width) / 2),
    [int](($screen.Height - $alarm.Height) / 2)
)

# Big label (fully opaque because alarm window is opaque)
$label = New-Object System.Windows.Forms.Label
$label.Text = "Feuerwehreinsatz!"
$label.ForeColor = [System.Drawing.Color]::White
# $label.BackColor = [System.Drawing.Color]::Red
$label.Font = New-Object System.Drawing.Font("Arial", 72,[System.Drawing.FontStyle]::Bold)
$label.Dock = 'Fill'
$label.TextAlign = 'MiddleCenter'
$label.AutoSize = $false
$label.Size = New-Object System.Drawing.Size($alarm.Width,300)
$label.Location = New-Object System.Drawing.Point(0,0)
$alarm.Controls.Add($label)

# Optional: click or any key to close the alarm
# $alarm.Add_KeyDown({ $alarm.Close(); $bg.Close() })
$alarm.Add_KeyDown({
    param($s,$e)
    if ($e.KeyCode -eq [System.Windows.Forms.Keys]::Escape) {
        $alarm.Close()
        $bg.Close()
    }
})
$alarm.Add_Click({ $alarm.Close(); $bg.Close() })

# Show background then show alarm as a dialog owned by background (keeps alarm above)
$bg.Show()
# show alarm modally so script waits until closed
$alarm.ShowDialog($bg) | Out-Null