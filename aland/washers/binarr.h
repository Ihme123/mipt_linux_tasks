struct pair
{
	int n;
	int x;
};
struct binarr
{
	int size, end;
	struct pair *p;
};

//create new empty array;
struct binarr *arrCreate();

//find element with KEY=key, log(n)-speed
int arrFind(struct binarr *arr, int key);

//add element to the end of array,returns on success number of element in the array, -1 otherwise
int arrPush_back(struct binarr *arr, int n, int x);

//frees all memory occupied by array
void arrFree(struct binarr *arr);

//sort array with qsort
void arrSort(struct binarr *arr);

//read paires "x:y" from file to array. returns array size on succes, -1 otherwise
int arrRead(const char *filename, struct binarr *arr);
