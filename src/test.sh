#!/bin/sh

correct=true

echo -ne '                          (0%)\r'
echo -e '\tNITEMS = 10, NTHREADS = 4'
gcc -o ex2 ex2.c -DNITEMS=10 -DNTHREADS=4 -DSHOWDATA=0 -lpthread -w

for i in `seq 1 1000`
do
	if [[ "`./ex2`" != "Well done, the sequential and parallel prefix sum arrays match." ]] ; then
		$correct = false
		printf "Failed test: %s\n" "$i"
	fi
done

echo -e '#                      (5%)\r'
echo -e '\tNITEMS = 17, NTHREADS = 4'
gcc -o ex2 ex2.c -DNITEMS=17 -DNTHREADS=4 -DSHOWDATA=0 -lpthread -w

for i in `seq 1 1000`
do
	if [[ "`./ex2`" != "Well done, the sequential and parallel prefix sum arrays match." ]] ; then
		$correct = false
		printf "Failed test: %s\n" "$i"
	fi
done

echo -e '##                     (10%)\r'
echo -e '\tNITEMS = 10000000, NTHREADS = 32'
gcc -o ex2 ex2.c -DNITEMS=10000000 -DNTHREADS=32 -DSHOWDATA=0 -lpthread -w

for i in `seq 1 1000`
do
	if [[ "`./ex2`" != "Well done, the sequential and parallel prefix sum arrays match." ]] ; then
		$correct = false
		printf "Failed test: %s\n" "$i"
	fi
  if [[ $i == 1 ]]; then
    echo -ne '#####                  (25%)\r'
  fi
  if [[ $i == 100 ]]; then
    echo -ne '######                 (32%)\r'
  fi
  if [[ $i == 200 ]]; then
    echo -ne '########               (40%)\r'
  fi
  if [[ $i == 300 ]]; then
    echo -ne '#########              (47%)\r'
  fi
  if [[ $i == 400 ]]; then
    echo -ne '###########            (55%)\r'
  fi
  if [[ $i == 500 ]]; then
    echo -ne '############           (62%)\r'
  fi
  if [[ $i == 600 ]]; then
    echo -ne '##############         (70%)\r'
  fi
  if [[ $i == 700 ]]; then
    echo -ne '###############        (77%)\r'
  fi
  if [[ $i == 800 ]]; then
    echo -ne '#################      (85%)\r'
  fi
  if [[ $i == 900 ]]; then
    echo -ne '##################     (92%)\r'
  fi
done

echo -e '####################   (100%)\r'
echo -ne '\n'

if $correct ; then
	echo 'All tests passed!'
else
	echo 'Tests failed!'
fi
