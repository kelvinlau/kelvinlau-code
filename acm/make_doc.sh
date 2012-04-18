version="$2 "
echo "ACM/ICPC templates $version"
echo "  by kelvinlau"
echo "  at `date '+%b %d, %Y'`"
echo
echo "menu:"
filenames=$1/*

for i in $filenames; do
  echo "$i" | sed 's/^.*\//  /'
done

for i in $filenames; do
  j=`echo "$i" | sed 's/^.*\///'`
  echo
  echo "/***** $j *****/"
  echo
  cat $i
done
