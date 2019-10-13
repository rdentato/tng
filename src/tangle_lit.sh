# (text:tng.md)
#
# ## Introduction
#
#   This is the *tangle* tool for a very simple literate programming
# system called `ltp`. I will assume familitarity with literate
# programming and will just recall some basic concepts. See the
# [Wikipedia](https://en.wikipedia.org/wiki/Literate_programming)
# article for more information on Literate Programming.
#
#   `ltp` deviates from the traditional Knuth's WEB style and has been mainly
# inspired to me by a post on
# [Kartik Agaram's blog](http://akkartik.name/post/literate-programming).
#
#   I do not subscribe entirely to Kartik's point of view on how to
# rearrange code to make it more easily understood (actually I find it
# more confusing), but his idea of using the concept of *waypoints*
# instead of *macros* for literate programming, immediately resonated with
# me as *"the right way to do it"*.
#
#   We can say this version of tangle is pretty aligned with the traditional
# literate programming view.
#
#   To me, the key points are:
#
#     - Source code must be readable in their native, unwaved form
#     - tangle helps keep close, in the file, elements that are
#       logically related so that their interaction can be explained
#       (and understood) more easily.
#
# ## Syntax
#  
#   While the tool pose no restriction on the syntax, it is assumed that
# the file will retain a structure that makes it a more or less valid source
# file for the language at hand.
#   It might not actually run/compile in its untangled form, but its look 
# should be familiar to the reader and, even more importantly, should not
# confuse editors and IDEs that will still be able to recognize lexical
# element in the code and provid their support.
#   Text is supposed to be in *comment* sections.
#
#   The directives are in the form: `(<directive>:<arg>)` and there
# are five of them:
#
#    - `(code:[filename])` What follows will be included in the generated file.
#                          The filename 
#    - `(text:[filename])`
#    - `(after:<waypoint>)`
#    - `(before:<waypoint>)`
#    - `(:<waypoint>)`
#
#   By the way, those above won't be recognized as directives since they are 
# within quotes.
#
#   There is no limitation no the way you write your *text* sections.
# Remember that the assumption is that the file will be mostly used in its
# raw, untangled form. Your effort should be to make it clear in that form,
# Here I've used mostly MarkDown syntax since many programmers are already
# familiar with that syntax. 

# ## The `tangle` script
#
#   To keep things simple, `tangle` it has been developed as a bash script.
# This sections provides a very high level of the script.

# NOTE: The script starts here! 
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv

# (code:tng.sh)
#!/usr/bin/bash

# (:Global Declaration)
# (:Regex for recognizing directives)  
# (:Functions)

#   Actually the logic is almost trivial.
#   The `tangle` function takes care of the entire process:
tangle () {
  # Remove leftovers from previous run just to stay safe
  # (:Remove temp files)
  
  # (:Handle command line options)  
  # (:Check that all files exist)
  # (:Split the chunks)
  # (:Reassemble files)

  # Cleanup temp files
  # (:Remove temp files)
}

tangle "$@"

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^
# NOTE: The script ends here!

# (text:)
# ## Splitting the file in chunks
#
#   This is the core of the first pass: each file is parsed and the
# directives are used to separate the pieces.
#
# (after:Split the chunks)
for fname in "$@" ; do splitchunks ; done

#  The `splitchunks` functions do the grunt work. It will parse the
# source file and split it in a set of files. There are three types
# of temporary files: 
#  
#  -  `~A~xxxx` Contains everything that should appear
#               after the waypoint xxx.
#  -  `~B~xxxx` Contains everything that should appear
#               before the waypoint xxx.
#  -  `~C~yyyy` What is left in the source file yyyy 
#               after the after/before parts have been 
#               moved to the other files.
#
# (after: Functions)
splitchunks () {
  bname=${fname##*/}  # faster than `basename $fname`
  cname="~C~$bname"
  curfile=$cname
  # (:Initialize line number)
  prtlnum
  while IFS= read -r line || [[ -n "$line" ]] ; do
    lnum+=1
    directive="~"
    # Set `directive`, `args` and `chunk` if there is a directive in the line
    # (:Check directive in line) 
    case $directive in
         after) curfile="~A~$chunk" ; prtlnum ;;
        before) curfile="~B~$chunk" ; prtlnum ;;
          code) curfile="$cname"    ; prtlnum 
                # (:Check output filename)
                ;;
          text) curfile="~" ;;
            "") echo "$line" >> "$curfile" ; prtlnum ;;
           "~") if [[ "$curfile" != "~" ]] ; then echo "$line" >> "$curfile" ; fi ;;
             *) error "Unknown directive '$directive'" ;;
    esac    
  done < "$fname"
}

# (text:)
# ### Setting output file name
#
#   By default, given the file `foo.x`, `tangle` will generate the 
# file `~foo.x` as output. This can be changed by specifying in one
# (and only one!) `(code:)` directive the name of the output filename.
#
#   To ensure there is only one of such directives, we record the line
# number where the output file name has been defined for the first time
# in the variable `outln`.

# (after:Global Declaration)
declare -g -i outln

# (after: Initialize line number)
#   As long as `outln` is negative, we know that the output file name has
# not been sepcified yet.
outln=-1

# (after:Check output filename)
#
if [[ ! -z $chunk ]] ; then
  if (( $outln < 0 )) ; then   # Still negative?
    echo "$line" >> "$curfile" # save it for the second pass
    outln=$((lnum-1))          # remember where has been set
  else
    error "Output file name already defined at line $outln"
  fi
fi

# ## Reassembling the files
#
# (after:Reassemble files)
for fname in "$@" ; do reassemble  ; done
  
# (after: Functions)
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
      # Set `directive`, `args` and `chunk` if there is a directive in the line
      # (:Check directive in line) 
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

# ## Recognizing directives
#
#   To check for a directive in a line we'll use the bash regular
# expression matching operator: `=~` that, if the match is successful,
# sets the `BASH_REMATCH[]` element to the captured submatches.

# (after:Regex for recognizing directives)  
#    v-------------------------------------------------------
tab='	'  # BEWARE THE LITERAL TAB within quotes ('\x09')!!!!
#    ^-------------------------------------------------------

quote='["`'"'"']'  # <-- This confusing sequence of quotes is just ['"`]

rxdirective='('$quote'?)\(([a-z]*):[ '$tab']*([A-Za-z0-9_. '$tab']*)\)'

#   The regex itself is a little bit tricky. There are three submatches:
#     - [1] a quoting character (single quote, double quote and backtick)
#     - [2] the directive (only lowercase characters)
#     - [3] a string of characters (only alphanumeric, `.`, `_` and spaces allowed )
#
#   The submatch [1] is used to allow the inclusion of a directive-like string in
# the source file without it being treated as such. For example: `(a:b)` is not
# recognized as a directive.
#
#   The submatch [2] is the directive. Note that there must be no spaces after
# the `(` or before the `:` while there can be spaces after the `:`.
# It will be empty for waypoints.
#
#   Submatch [3] is the trickiest. It will be used as the *chunk name* (see the
# section "Handling chunk names" below) or by the `code:` directive to set the
# output file name (see the section "Setting output file name" above)

# (after:Check directive in line)
if [[ "$line" =~ $rxdirective && -z "${BASH_REMATCH[1]}" ]] ; then
  directive="${BASH_REMATCH[2]}"
  args="${BASH_REMATCH[3]}"
  getchunkname "$args"   # handle chunk name
fi

# ### Handling chunk names
#
# (after: Global Declaration)
shopt -s extglob

# (after:Global Declaration)
declare -g -l chunk

# (after: Functions)
getchunkname () {
  #chunk=$(echo "$1" | sed -e 's/\s\s*/~/g' -e 's/~~*$//' )
  chunk="$1"
  chunk=${chunk%%+([[:space:]])}
  chunk=${chunk//+([[:space:]])/'~'}
}


# ## Print `#line` 
#
#   As it is customary for preprocessors, tangle will insert into 
# the generated code a set of `#line` directives that will allow 
# the compiler/interpreter to signal any error by referring to the
# original source file rather than the generated file.
#
#  The current line number is kept in the `lnum` variable
# (after:Global Declaration)
declare -g -i lnum

# (:Initialize line number)
#   During the split phase, the line number will be initialized 
# for each file
lnum=1

#   The generation of `#lines` directives can be turned off with
# a the command line option `-n`.
#
# (after:Global Declaration)
  declare -g -i prtln ; prtln=1  # 1: print it  0: don't

# (after:Handle command line options)  
if [[ "$1" == "-n" ]] ; then
  prtln=0 ; shift
fi

# (after: Functions)
prtlnum () {
  if (( $prtln )) ; then
    echo "#line $lnum \"$bname\"" >> "$curfile"
  fi
}

# ## Cleanup
#
# (after: Remove temp files)
rm -f \~[ABC]~*

# ## Error Handling
#
# The `error` function
# (after: Functions)
error () {
  echo "ERROR: $1. $fname:$((lnum-1))" 1>&2 
  cleanup
  exit
}

# (after:Check that all files exist)
for fname in "$@" ; do
  if [[ ! -f "$fname" ]] ; then
    echo "Missing file: $fname" 1>&2 ; exit
  fi
done


