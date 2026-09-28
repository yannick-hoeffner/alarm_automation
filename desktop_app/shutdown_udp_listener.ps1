$event = [System.Threading.EventWaitHandle]::OpenExisting("UDP-ALARM-LISTENER-SHUTDOWN-EVENT")
$event.Set()
$event.Close() # close does not unset the event, it just releases the handle