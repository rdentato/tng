# (text:tng.md)
#
# ## Introduction
#
#   This is the *tangle* tool for a very simple literate programming
# system called `tng`. I will assume familitarity with literate
# programming and will just recall some basic concepts. See the
# [Wikipedia](https://en.wikipedia.org/wiki/Literate_programming)
# article for more information on Literate Programming.
#
#   `tng` deviates from the traditional Knuth's WEB style and has been mainly
# inspired to me by a post on
# [Kartik Agaram's blog](http://akkartik.name/post/wart-layers).
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

# ## Version
#   Useful to check the version in use
# (after:Global Declaration)
tng_ver=0x0001004F
tng_ver_str="0.1.4"

# ## The `tangle` script
#
#   To keep things simple, `tangle` it has been developed as a bash script.
# This sections provides a very high level of the script.

# NOTE: The script starts here! 
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvv

# (code:tng.sh)
#!/usr/bin/env bash
#: tng (c) 2019 Remo Dentato
#: https://github.com/rdentato/tng
#

# (:Global Declaration)
# (:Regex for recognizing directives)  
# (:Functions)

#   Actually the logic is almost trivial.
#   The `tangle` function takes care of the entire process:

tangle () {
  # Remove leftovers from previous run just to stay safe
  # (:Remove temp files)
  
  # (:Check that all files exist)
  # (:Split the chunks)
  # (:Reassemble files)

  # Cleanup temp files
  # (:Remove temp files)
}
# (:Get options) 
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
    # (:Check directive in line) 
    case $directive in
                # (:Handle void directive)
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

rxdirective='('$quote'?)\('$quote'?([a-z]*):[ '$tab']*([A-Za-z0-9_. '$tab']*)'$quote'?\)'

# (:)
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
checkdirective

# (after: Functions)
checkdirective () {
  if [[ "$line" =~ $rxdirective && -z "${BASH_REMATCH[1]}" ]] ; then
    directive="${BASH_REMATCH[2]}"
    args="${BASH_REMATCH[3]}"
    getchunkname "$args"   # handle chunk name
  fi
  if [[ -z $chunk && -z $directive ]] ; then directive="text" ; fi
  # (:Handle void)
}
# (:)

# ### Handling chunk names
#
#   To enable some expressiveness, we must give flexibility on the waypoint
# (and, hence, chunks) definition. Not too much, or it wil bee to complicated
# to handle (in the end they will be used as file names), but not to little 
# that the programmer has to remember exactly how she spelled this or that waypoint.
#
#   Chunk names are transformed according the following rulese:
#     - Letters are converted to lower case
#     - Tabs are converted in spaces
#     - Trailing and leading spaces are removed
#     - Sequence of spaces are collapsed into a single space
#     - Spaces are replaced with `~` (to create an easire to handle filename)
#
#   All the following directives are the same:
#     -  `(after: Do fancy stuff)`
#     -  `(after:Do fancy stuff)`
#     -  `(after: Do Fancy STUFF)`
#     -  `(after: do   Fancy    stuff   )`
#     -  `(after: do   fancy    stuff   )`
#
# but they are not equivalent to any of these:
#     -  `(after: Do fancy stuff.)`    <- there is a period at the end
#     -  `(after: D o fan cy stu ff)`  <- there a space between `D` and `o`

# (after:Global Declaration)
# The `-l` option will convert the content to lower case
declare -g -l chunk

# (after: Functions)
getchunkname () {
  # chunk=$(echo "$1" | sed -e 's/\s\s*/~/g' -e 's/~~*$//' )
  chunk="$1"
  chunk=${chunk%%+([[:space:]])}      # <---+- thanks to shopt -s below
  chunk=${chunk//+([[:space:]])/'~'}  # <--/
}

# (after: Global Declaration)
# This enables the extended matching for Bash string substitution.
shopt -s extglob


# ### Void
#   It is sometimes useful to have a way to suspend the interpretation
# of tangling directives.
# This can be done by using `(void:xxx)` that will make transparent
# all the lines untile another `(void:xxx)` is encountered (with the 
# *same* string `xxx`, of course)
#
#   We'll hold the string that has been specified in the `void` variable.

# (after: Handle void)
  if [[ -z $void ]] ; then
    #   If it's empty, it means that we are not *"in the void"*, so we check
    # if the `(void:xxxx)` directive has been 
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

# (after:Handle void directive)
#   When in a *void* we still have to print an empty line to avoid messing up
# the line numbering.

  void) if [[ "$curfile" != "~" ]] ; then echo >> "$curfile" ; fi ;;

# (:)


# ## Printing `#line` 
#
#   As it is customary for preprocessors, tangle will insert into 
# the generated code a set of `#line` directives that will allow 
# the compiler/interpreter to signal any error by referring to the
# original source file rather than the generated file.
#
#  The current line number is kept in the `lnum` variable
# (after:Global Declaration)
declare -g -i lnum

# (after:Initialize line number)
#   During the split phase, the line number will be initialized 
# for each file
lnum=1

#   The generation of `#lines` directives can be turned off with
# a the command line option `-n`.
#
# (after:Global Declaration)
declare -g -i prtln ; prtln=1  # 1: print it  0: don't

# (after: Functions)
prtlnum () {
  if (( $prtln )) ; then
    echo "#line $lnum \"$bname\"" >> "$curfile"
  fi
}

# ## Cleanup
#
# (after: Remove temp files)
rm -f \~[ABC]~* 2> /dev/null

# ## Error Handling
#
# The `error` function
# (after: Functions)
error () {
  echo "Error: $1. $fname:$((lnum-1))" 1>&2 
  # (:Remove temp files)
  exit
}

# (after:Check that all files exist)
for fname in "$@" ; do
  if [[ ! -f "$fname" ]] ; then
    echo "Missing file: $fname" 1>&2 ; exit
  fi
done

# ## Options
# (after: Get options)
  opts=1
  while [[ $opts = 1 ]] ; do
    case "$1" in 
          -h)  usage 0 ;; 
          -n)  prtln=0 ; shift ;;
     "" | -?)  usage 1 ;;
           *)  opts=0 ;; 
    esac 
  done

# (after: Global functions)

usage () {
  echo "tng [-n] file [file ...]" 1>&2
  echo "Version: $tng_ver_str ($tng_ver)" 1>&2
  echo "Options:   -n   no #line directives" 1>&2
  echo "           -h   help" 1>&2
  exit $1
}

