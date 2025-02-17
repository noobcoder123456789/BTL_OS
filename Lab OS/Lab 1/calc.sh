while [[ true ]]
do
   read -p ">> " str

   if [[ ${str:0:1} = "E" ]]
   then
      exit 0
      continue
   fi

   if [[ ${#str} = 0 ]]
   then
      clear
   fi

   if [[ ${str:0:1} = "H" ]]
   then
      cat save.txt
      continue
   fi

   len=$((${#str}-1))

   cong=0
   tru=0
   nhan=0
   chia=0
   du=0

   x="0"
   y="0"

   TMPFILE=./temp.txt
   if test -f "$TMPFILE"
   then
      ANS="`cat ./temp.txt`"
   else
      ANS=0
   fi

   for (( i=0; i<=len; i++ ))
   do   
      if [[ ${str:i:1} = "+" ]]
      then
         cong=1
         x=${str:0:i}
         y=${str:i+2:len-i-1}
      fi
      
      if [[ ${str:i:1} = "-" ]]
      then
         tru=1
         x=${str:0:i}
         y=${str:i+2:len-i-1}
      fi

      if [[ ${str:i:1} = "*" ]]
      then
         nhan=1
         x=${str:0:i}
         y=${str:i+2:len-i-1}
      fi

      if [[ ${str:i:1} = "/" ]]
      then
         chia=1
         x=${str:0:i}
         y=${str:i+2:len-i-1}
      fi

      if [[ ${str:i:1} = "%" ]]
      then
         du=1
         x=${str:0:i}
         y=${str:i+2:len-i-1}
      fi
   done

   check=$((cong + tru + nhan + chia + du))
   if [[ $check = 0 ]]
   then
      echo "SYNTAX ERROR"
      continue
   fi

   if [[ ${x:0:1} = "A" ]]
   then
      x=$ANS
   fi

   if [[ ${y:0:1} = "A" ]]
   then
      y=$ANS
   fi

   if [[ $y = 0 ]]
   then
      echo "MATH ERROR"
      continue
   fi

   if [[ $cong = 1 ]] 
   then      
      ANS=$(echo "scale=2; $x + $y" | bc)
      echo $ANS
      echo "$x + $y = $ANS" >> "./save.txt"
      cnt=$(sed -n '$=' save.txt)
      if [[ $cnt = 6 ]]
      then
         sed -i '1d' save.txt
      fi
   fi

   if [[ $tru = 1 ]] 
   then   
      ANS=$(echo "scale=2; $x - $y" | bc)
      echo $ANS
      echo "$x - $y = $ANS" >> "./save.txt"
      cnt=$(sed -n '$=' save.txt)
      if [[ $cnt = 6 ]]
      then
         sed -i '1d' save.txt
      fi
   fi

   if [[ $nhan = 1 ]] 
   then   
      ANS=$(echo "scale=2; $x * $y" | bc)
      echo $ANS
      echo "$x * $y = $ANS" >> "./save.txt"
      cnt=$(sed -n '$=' save.txt)
      if [[ $cnt = 6 ]]
      then
         sed -i '1d' save.txt
      fi
   fi

   if [[ $chia = 1 ]] 
   then   
      ANS=$(echo "scale=2; $x / $y" | bc)
      echo $ANS
      echo "$x / $y = $ANS" >> "./save.txt"
      cnt=$(sed -n '$=' save.txt)
      if [[ $cnt = 6 ]]
      then
         sed -i '1d' save.txt
      fi
   fi

   if [[ $du = 1 ]] 
   then   
      ANS=$((x % y))
      echo "$ANS"
      echo "$x % $y = $ANS" >> "./save.txt"
      cnt=$(sed -n '$=' save.txt)
      if [[ $cnt = 6 ]]
      then
         sed -i '1d' save.txt
      fi
   fi

   echo "$ANS" > ./temp.txt
   read -rsn1
   clear
done