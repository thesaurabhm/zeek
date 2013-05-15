# @TEST-EXEC: bro -r $TRACES/http/206_example_a.pcap $SCRIPTS/file-analysis-test.bro %INPUT >a.out
# @TEST-EXEC: btest-diff a.out
# @TEST-EXEC: wc -c 7gZBKVUgy4l-file0 | sed 's/^[ \t]* //g' >a.size
# @TEST-EXEC: btest-diff a.size

# @TEST-EXEC: bro -r $TRACES/http/206_example_b.pcap $SCRIPTS/file-analysis-test.bro %INPUT >b.out
# @TEST-EXEC: btest-diff b.out
# @TEST-EXEC: wc -c oDwT1BbzjM1-file0 | sed 's/^[ \t]* //g' >b.size
# @TEST-EXEC: btest-diff b.size

# @TEST-EXEC: bro -r $TRACES/http/206_example_c.pcap $SCRIPTS/file-analysis-test.bro %INPUT >c.out
# @TEST-EXEC: btest-diff c.out
# @TEST-EXEC: wc -c uHS14uhRKGe-file0 | sed 's/^[ \t]* //g' >c.size
# @TEST-EXEC: btest-diff c.size

global cnt: count = 0;

redef test_file_analysis_source = "HTTP";

redef test_get_file_name = function(f: fa_file): string
	{
	local rval: string = fmt("%s-file%d", f$id, cnt);
	++cnt;
	return rval;
	};
