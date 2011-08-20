all:
	node-waf configure && node-waf build
	ln -s build/default/fork.node .
