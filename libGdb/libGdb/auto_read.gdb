set pagination off
set confirm off
set breakpoint pending on
set print elements 0
set print repeats 0
set print pretty off
set print symbol-filename off
set unwindonsignal on
break gdb_dump_read_date
commands
  silent
  printf "\n[GDB] === read condition ===\n"
  # C++側から直接文字列ポインタが渡ってくるので、そのまま %s で出力可能
  printf " cal_ym=%s\n", ym
  printf " heidokyu=%s\n", hdk
  printf " ymd=%s\n", ymd
  continue
end
break Cgt0927ChildResultKnr::deleteResults
commands
  silent
  printf "[GDB] === EVIDENCE: deleteResults() Executed ===\n"
  continue
end
break gdb_dump_out
commands
  silent
  printf "[GDB] --- out dump ---\n"
  set $n = (int)out.size()
  printf " out.size=%d\n", $n
  set $i = 0
  while ($i < $n)
    printf " rec[%d].oya_process_id=%d\n", $i, out[$i].oya_process_id
    printf " rec[%d].cal_result_flg=%d\n", $i, out[$i].cal_result_flg
    printf " rec[%d].cal_ym=\"%s\"\n", $i, (char*)&out[$i].cal_ym[0]
    printf " rec[%d].cal_heidokyu=\"%s\"\n", $i, (char*)&out[$i].cal_heidokyu[0]
    printf " rec[%d].cal_ymd=\"%s\"\n", $i, (char*)&out[$i].cal_ymd[0]
    set $i = $i + 1
  end
  continue
end

run
