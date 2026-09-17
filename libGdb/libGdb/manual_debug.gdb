set pagination off
set confirm off
set breakpoint pending on
set print elements 0
set print repeats 0
set print pretty off
set print symbol-filename off
set unwindonsignal on

# 1. 削除処理
break Cgt0927ChildResultKnr::deleteResults
commands
  silent
  printf "\n=== EVIDENCE: deleteResults() Executed ===\n"
  # ここで一時停止します (c で次へ)
end

# 2. read条件の確認 (pid, flg 含む)
break gdb_dump_read_date
commands
  silent
  printf "\n=== read condition ===\n"
  
  printf "(gdb) p pid\n"
  p pid
  
  printf "(gdb) p flg\n"
  p flg
  
  printf "(gdb) p ym\n"
  p ym
  
  printf "(gdb) p hdk\n"
  p hdk
  
  printf "(gdb) p ymd\n"
  p ymd
  
  # ここで一時停止します (c で次へ)
end

# 3. 結果のダンプ
break gdb_dump_out
commands
  silent
  printf "\n--- out dump ---\n"
  set $n = (int)out.size()
  printf "(gdb) p out.size()\n"
  p $n
  
  set $i = 0
  while ($i < $n)
    printf "\n[Record #%d]\n", $i
    
    printf "(gdb) p out[%d].oya_process_id\n", $i
    p out[$i].oya_process_id
    
    printf "(gdb) p out[%d].cal_result_flg\n", $i
    p out[$i].cal_result_flg
    
    # 複数一致の差異確認用として追加
    printf "(gdb) p out[%d].subfile_no\n", $i
    p out[$i].subfile_no
    
    printf "(gdb) p (char*)&out[%d].cal_ym[0]\n", $i
    p (char*)&out[$i].cal_ym[0]
    
    printf "(gdb) p (char*)&out[%d].cal_heidokyu[0]\n", $i
    p (char*)&out[$i].cal_heidokyu[0]
    
    printf "(gdb) p (char*)&out[%d].cal_ymd[0]\n", $i
    p (char*)&out[$i].cal_ymd[0]
    
    set $i = $i + 1
  end
  # ここで一時停止します (c で次へ)
end

run
