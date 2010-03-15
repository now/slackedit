# a simple and stupid script to autoincrement build information
# for msdev projects since it can't be done right with msdev macros

proc open_version_file { } {
  if { [catch {open "[pwd]/version_info.h" r} fileid] } {
    set fileid [open "[pwd]/version_info.h" w]
    puts $fileid {#define FILEVER         1.0.0.0}
    puts $fileid {#define PRODUCTVER      1.0.0.0}
    puts $fileid {#define BUILDCOUNTSTR   _T("0")}
    puts $fileid {#define BUILDDATE       _T("00/00/00")}
    close $fileid
    return [open "[pwd]/version_info.h" r]
  } else {
    return $fileid
  }
}

set fileid [open_version_file]
set line [gets $fileid]
set lineparts [split $line "."]
set newline1 [join [lreplace $lineparts end end [expr {[lindex $lineparts end] + 1}]] "."]
set line [gets $fileid]
set lineparts [split $line "."]
set newline2 [join [lreplace $lineparts end end [expr {[lindex $lineparts end] + 1}]] "."]
set line [gets $fileid]
set lineparts [split $line {"}]
set newline3 [join [lreplace $lineparts 1 1 [expr {[lindex $lineparts 1] + 1}]] {"}]
set line [gets $fileid]
set lineparts [split $line {"}]
set newline4 [join [lreplace $lineparts 1 1 [clock format [clock seconds] -format "%m/%d/%y"]] {"}]
close $fileid
set fileid [open "[pwd]/version_info.h" w]
puts $fileid $newline1
puts $fileid $newline2
puts $fileid $newline3
puts $fileid $newline4
close $fileid
