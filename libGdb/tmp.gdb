set pagination off
set confirm off
set breakpoint pending on
set print pretty on
set print elements 0

# --------------------------------------------------
# write 側
# --------------------------------------------------
break gdb_dump_write
commands
  silent
  printf "\n=== Break: gdb_dump_write ===\n"
  frame
  print ret_val
  continue
end

# --------------------------------------------------
# read 前
# --------------------------------------------------
break gdb_dump_read_before
commands
  silent
  printf "\n=== Break: gdb_dump_read_before ===\n"
  frame
  continue
end

# --------------------------------------------------
# read 後
# --------------------------------------------------
break gdb_dump_read_after
commands
  silent
  printf "\n=== Break: gdb_dump_read_after ===\n"
  frame
  print ret_val
  continue
end

# --------------------------------------------------
# read 結果
# --------------------------------------------------
break gdb_dump_read
commands
  silent
  printf "\n=== Break: gdb_dump_read ===\n"
  frame
  print ret_val
  up
  printf "\n--- out ---\n"
  print out
  continue
end

# --------------------------------------------------
# readRecords 内部
# --------------------------------------------------
break gdb_dump_read_records
commands
  silent
  printf "\n=== Break: gdb_dump_read_records ===\n"
  frame
  up
  printf "\n--- readRecords locals ---\n"
  print sOyaPid
  print vKey
  print vKeyData

  # 追加で確認したいもの
  printf "\n--- prm keys ---\n"
  print vPrmKey
  print vPrmKeyData
  print vOutData
  print vOutRecNo
  continue
end

# --------------------------------------------------
# readRecords 後
# --------------------------------------------------
break gdb_dump_read_records_after
commands
  silent
  printf "\n=== Break: gdb_dump_read_records_after ===\n"
  frame
  up
  printf "\n--- after SdmReadRecByKey ---\n"
  print ret
  print vOutData
  print vOutRecNo
  continue
end

# --------------------------------------------------
# 必要なら main 側で stop している場合に再開
# --------------------------------------------------
continue
