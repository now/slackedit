# copy necessary dlls to necessary path

set dlls [list pcp_controls pcp_edit pcp_generic pcp_paint]
set root "c:/my documents/my source code"
set newroot "$root/SlackEdit"
set command [lindex $argv 0]

if { ($argc == 0) || (($command != "debug") && ($command != "release")) } {
	puts "$command: <debug|release>"
	exit 1
}

foreach dll $dlls {
	file copy -force "$root/$dll/$command/$dll.dll" "$newroot/output/$command/$dll.dll"
}
