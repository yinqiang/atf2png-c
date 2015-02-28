# atf2png-c
Uncompress ATF v6 file to png.<br/>
But it just can export ETC1 pkm file now.

###Format support:
1. ATFRAWCOMPRESSEDALPHA

###Usage:
```shell
$ make
$ ./atf2png path_to.atf  # export output.pkm and output_alpha.pkm
```

# mergetool
Merge two png files into one.

###Usage:
```shell
mergetool -i inputdir -o outputdir
```