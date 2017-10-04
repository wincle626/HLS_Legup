source ../config.tcl

# temporarily disable local rams
# points-to analysis has trouble with:
#       %1 = call i8* bitcast (void (i8*, i8, i32)* @legup_memset_4 to i8* (i8*, i8, i64)*)(i8* bitcast ([24 x i32]* @tqmf to i8*), i8 0, i64 96) #2
#set_parameter LOCAL_RAMS 0

