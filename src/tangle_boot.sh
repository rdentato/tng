#!/usr/bin/bash
tng_ver=0x0001003F
tng_ver_str="0.1.3"

declare -g -i prtln   ; prtln=1
declare -g -i outln
declare -g -i lnum
declare -g -l chunk

shopt -s extglob

usage () {
  echo "tng [-n] file [file ...]" 1>&2
  echo "Version: $tng_ver_str ($tng_ver)" 1>&2
  echo "Options:   -n   no #line directives" 1>&2
  echo "           -h   help" 1>&2
  exit $1
}

cleanup () {
  rm -f \~[ABC]~*  2> /dev/null
}

error () {
  echo "Error: $1. $fname:$((lnum-1))" 1>&2 
  cleanup
  exit
}

prtlnum () {
  if (( $prtln )) ; then
    echo "#line $lnum \"$bname\"" >> "$curfile"
  fi
}

# Set `directive`, `args` and `chunk` if there is a directive in the line
checkdirective () {
  if [[ "$line" =~ $rxdirective && -z "${BASH_REMATCH[1]}" ]] ; then
    directive="${BASH_REMATCH[2]}"
    args="${BASH_REMATCH[3]}"
    getchunkname "$args"
  fi
  if [[ -z $chunk && -z $directive ]] ; then directive="text" ; fi
  if [[ -z $void ]] ; then
    if [[ "$directive" = "void" ]] ; then
      if [[ -z $args ]] ; then error "Invalid '(void:xxx)' directive'" ; fi
      void=$args
    fi
  else
    if [[ "$directive" = "void" && "$void" = "$args" ]] ; then
      void=""
    fi
    directive="void"
  fi
}

getchunkname () {
  #chunk=$(echo "$1" | sed -e 's/\s\s*/~/g' -e 's/~~*$//' )
  chunk="$1"
  chunk=${chunk%%+([[:space:]])}
  chunk=${chunk//+([[:space:]])/'~'}
}

splitchunks () {
  bname=${fname##*/}  #`basename $fname`
  cname="~C~$bname"
  curfile=$cname
  lnum=1
  outln=-1
  void=""
  prtlnum
  while IFS= read -r line || [[ -n "$line" ]] ; do
    lnum+=1
    directive="~"
    checkdirective
    case $directive in
          void) # write an empty line to avoid messing up with line numbering
                if [[ "$curfile" != "~" ]] ; then echo >> "$curfile" ; fi ;;
         after) curfile="~A~$chunk" ; prtlnum ;;
        before) curfile="~B~$chunk" ; prtlnum ;;
          code) curfile="$cname"    ; prtlnum 
                if [[ ! -z $chunk ]] ; then
                  if (( $outln < 0 )) ; then 
                    echo "$line" >> "$curfile" ;  outln=$((lnum-1))
                  else
                    error "Output file name already defined at line $outln"
                  fi
                fi
                ;;
          text) curfile="~" ;;
            "") echo "$line" >> "$curfile" ; prtlnum ;; # Target
           "~") if [[ "$curfile" != "~" ]] ; then echo "$line" >> "$curfile" ; fi ;;
             *) error "Unknown directive '$directive'" ;;
    esac
  done < "$fname"
}

reassemble () {
  bname=${fname##*/}  #`basename $fname`
  dname="${fname%$bname}" # "`dirname $1`/~$bname"
  bname="~$bname"
  cname="~C$bname"
  declare -i doreassemble; doreassemble=1
  declare -i numpass; numpass=0
  while (( $doreassemble && $numpass < 10 )) ; do
    doreassemble=0
    numpass+=1
    while IFS= read -r line || [[ -n "$line" ]] ; do
      directive="~"
      checkdirective
      case $directive in 
         "" ) if [[ -f "~B~$chunk" ]] ; then cat "~B~$chunk" ; fi
              if [[ -f "~A~$chunk" ]] ; then cat "~A~$chunk" ; fi
              doreassemble=1
              ;;
         "~") echo "$line" ;;
        code) bname="$args" ;;
      esac
    done < "$cname" > "~C~"
    mv "~C~" "$cname"
  done
  mv "$cname" "$dname$bname"
}

tangle () {

  #    v-------------------------------------------------------
  tab='	'  # BEWARE THE LITERAL TAB within quotes ('\x09')!!!!
  #    ^-------------------------------------------------------
  quote='["`'"'"']'
  rxdirective='('$quote'?)\('$quote'?([a-z]*):[ '$tab']*([A-Za-z0-9_. '$tab']*)'$quote'?\)'

  # Remove leftovers from previous run
  cleanup
  
  for fname in "$@" ; do
    if [[ ! -f "$fname" ]] ; then
      echo "Missing file: $fname" 1>&2 ; exit
    fi
  done
  
  for fname in "$@" ; do splitchunks ; done
  for fname in "$@" ; do reassemble  ; done
  
  # Cleanup temp files
  cleanup
}

opts=1
while [[ $opts = 1 ]] ; do
  case "$1" in 
        -h)  usage 0 ;; 
        -n)  prtln=0 ; shift ;;
   "" | -?)  usage 1 ;;
         *)  opts=0 ;; 
  esac 
done

tangle "$@"
