set pagination off
set confirm off
set print pretty on
set print elements 0
set print null-stop on
set language c++
set breakpoint pending on

# 試験項目ごとにブレイクポイントを張り、エビデンスを出力する
break gdb_dump_out
commands
  silent
  # 試験名・件数
  printf "\n=== TEST: %s ===\n", label
  set $n = (int)out.size()
  set $sec_count = (int) gdb_get_sec_count()
  printf "--- out dump (size=%d, sec_count=%d) ---\n", $n, $sec_count

  set $i = 0
  while ($i < $n)
    printf "\n#%d\n", $i

    # 数値系
    p out[$i].oya_process_id
    p out[$i].ko_pidx
    p out[$i].subfile_flg
    p out[$i].subfile_no
    p out[$i].cal_result_flg

    # 文字系
    printf ".ko_proc_name:     \"%s\"\n", out[$i].ko_proc_name
    printf ".cal_ym:           \"%s\"\n", out[$i].cal_ym
    printf ".cal_heidokyu:     \"%s\"\n", out[$i].cal_heidokyu
    printf ".cal_ymd:          \"%s\"\n", out[$i].cal_ymd

    set $i = $i + 1
  end

  continue
end

# 削除処理（クリーンアップ）のエビデンス用ブレイクポイント
break Cgt0927ChildResultKnr::deleteResults
commands
  silent
  printf "\n=== EVIDENCE: Cgt0927ChildResultKnr::deleteResults() Executed ===\n"
  continue
end

run