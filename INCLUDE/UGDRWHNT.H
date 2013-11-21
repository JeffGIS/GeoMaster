
//drawing hint linked list structure
typedef struct UGDrwHintListTag{
	int startCol;
	long startRow;
	int endCol;
	long endRow;
	
	UGDrwHintListTag * next;

}UGDrwHintList;

typedef struct UGDrwHintVListTag{
	int Col;
	long Row;
	
	UGDrwHintVListTag * next;

}UGDrwHintVList;


//drawing hint class
class CUGDrawHint{

private:

	UGDrwHintList * m_List;
	UGDrwHintVList * m_VList;

public:
	CUGDrawHint();
	~CUGDrawHint();

	void AddHint(int col,long row);
	void AddHint(int startCol,long startRow,int endCol,long endRow);
	void ClearHints();

	int IsInvalid(int col,long row);
	int IsValid(int col,long row);
	int GetTotalRange(int *startCol,long *startRow,int *endCol,long *endRow);

	void SetAsValid(int col,long row);
};