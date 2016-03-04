ADD_TEST(mex "./mleval" "run test_mex.m" "-s")

ADD_TEST(local_init "./mleval" "run test_local_init.m" "-s")
ADD_TEST(local_open_close "./mleval" "run test_local_open_close.m" "-s")
ADD_TEST(local_version "./mleval" "run init_local; run test_version.m" "-s")
ADD_TEST(local_read "./mleval" "run init_local; run test_read.m" "-s")
ADD_TEST(local_write "./mleval" "run init_local; run test_write.m" "-s")
ADD_TEST(example "./mleval" "run init_local; run ../example/example.m" "-s")

ADD_TEST(remote_init "./mleval" "run test_remote_init.m" "-s")
#the init remote automatically checks the version. test_version only checks the local version, thus was duplicate
ADD_TEST(remote_read "./mleval" "run init_remote; run test_read.m" "-s")
ADD_TEST(remote_write "./mleval" "run init_remote; run test_write.m" "-s")


