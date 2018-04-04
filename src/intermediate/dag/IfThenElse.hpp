struct IfThenElse : public Node {
	DECLARE_ACCEPT
	Node condition;
	Node if_block;
	Node else_block;
};