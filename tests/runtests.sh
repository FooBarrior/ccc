for i in $@; do
    echo "Now Testing \"$i\""
    for j in $i/*.t; do
        tname=${j%'.t'}
        echo -n "${tname#$i/}: "
        if [ ! -e $tname.a ]; then
            echo "answer file not found!\n"
        else
            ./$i.run $j > $j.a
            if [ $? -ne 0 ]; then
                echo "epic fail!"
            else
                res=`cmp $tname.a $j.a`
                if [ $? -ne 0 ]; then
                    echo "test failed - $res\nRun external diff: "
                    read ediff
                    $ediff $tname.a $j.a
                else echo "success"; fi
            fi
            rm $j.a > /dev/null
        fi
    done
    echo "Done testing $i"
done
