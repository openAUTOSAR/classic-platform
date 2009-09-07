
define test
  print &$arg0
  print &($arg0.ready_list)
  print $arg0.ready_list
end

echo \n\n
set print address on
print &k_sys.ready_head
print k_sys.ready_head
test pcb_list[0]
test pcb_list[1]

#printf "k_sys.ready_head(location)=0x%x\n",&k_sys.ready_head
#printf "pcb_list[0](loc)=0x%x\n",(&((pcb_t *)pcb_list)[0])
#print (&((pcb_t *)pcb_list)[0])->ready_list
#printf "pcb_list[1](loc)=0x%x\n",(&((pcb_t *)pcb_list)[1])
#print (&((pcb_t *)pcb_list)[1])->ready_list

#echo \n
#output k_sys.ready_head
#echo \npcb_list[0]\n
#echo &pcb_list[0]\n
#output (&((pcb_t *)pcb_list)[0])
#echo \nready_list\n
#output &(&((pcb_t *)pcb_list)[0])->ready_list
#echo \n
#output (&((pcb_t *)pcb_list)[0])->ready_list

#echo \n\n&pcb_list[1]\n
#output (&((pcb_t *)pcb_list)[1])
#echo \nready_list\n
#output &(&((pcb_t *)pcb_list)[1])->ready_list
#echo \n
#output (&((pcb_t *)pcb_list)[1])->ready_list





