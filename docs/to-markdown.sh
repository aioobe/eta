awk '$1 == ".TH" { next }
     $1 == ".RS" { codeMode=1; next }
     $1 == ".RE" { codeMode=0; next }
     $1 == ".TP" { tpMode=1; print ""; next }
     tpMode == 1 && $1 == ".B" { $1=""; printf "- `%s`\n\n",$0; tpMode=2; next }
     tpMode == 2 { printf "  %s",$0; tpMode=0; next }
     codeMode && $1 == ".B" { $1=""; print "   " $0; next }
     $1 == ".br" { if (codeMode) { printf "" } else { print "\n\n" }; next }
     codeMode { print "    " $0; next }
     codeMode && $1 == ".B" { $1=""; print "   " $0; next }
     $1 == ".SH" { $1 = ""; print "##" $0; next }
     $1 == ".SS" { $1 = ""; print "###" $0; next }
     $1 == ".B" { printf "`"; for (i=2; i <= NF; i++) { if (i > 2) {printf " ";} printf $i; } print "`"; next }
     $1 == ".BR" { printf "`%s`",$2; $1 = $2 = ""; print $0; next }
     { print $0 }' < eta.1 \
         | sed 's/\\fI\([^ ]*\)\\fR/`\1`/g' \
         | sed 's/\\fB\([^ ]*\)\\fR/`\1`/g' \
         | sed 's/\\\[u2026\]/…/g' \
         | sed 's/\\ / /g' \
         | sed 's/\\-/-/g' \
         | sed 's/"""/"/g'

