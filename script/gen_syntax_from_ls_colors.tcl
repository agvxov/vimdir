#!/usr/bin/tclsh

if {[info exists env(LS_COLORS)] == 0} {
    puts stderr "Error: LS_COLORS environment variable is not set."
    exit 1
}

set rules {}
set lscolors $env(LS_COLORS)
foreach pair [split $lscolors :] {
    set color [lindex [split $pair =] 1]
    set pattern [lindex [split $pair =] 0]
    lappend rules [list $pattern $color]
}

proc to_clean_name {string} {
    set r $string
    set r [regsub -all {[^a-zA-Z0-9]} $r ""]
    return $r
}

proc rewrite_regex {string} {
    set r $string
    set r [string map {"*" ".\*"} $r]
    return $r
}

proc match_rule {rule} {
    set template "syn match vd%s \"%s\""
    set name [to_clean_name [lindex $rule 0]]
    set pattern [rewrite_regex [lindex $rule 0]]
    return [format $template $name $pattern]
}

proc def_rule {rule} {
    set template "hi def vd%s ctermfg=%s"
    set name [to_clean_name [lindex $rule 0]]
    set color [lindex $rule 1]
    return [format $template $name $color]
}

foreach rule $rules {
    puts [match_rule $rule]
}

foreach rule $rules {
    puts [def_rule $rule]
}
