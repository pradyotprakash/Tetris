//Written by Pradyot Prakash, IIT Bombay.

#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Box.H>
#include<FL/fl_draw.H>

#include<iostream>
#include<unistd.h>
#include<ctime>
#include<math.h>
#include<sstream>

using namespace std;

static const int tilesize = 30;
static const int xmaxtiles= 14;
static const int bgcolor= 92;
static const double timeout = 0.3;
static const int ymaxtiles= 20;

class Tile
{
	public : 
	int x,y;
};

class Board;

void ts_down (Tile ts[4]) 	{ for (int i=0; i<4; i++) ts[i].y++; }
void ts_right (Tile ts[4]) 	{ for (int i=0; i<4; i++) ts[i].x++; }
void ts_left (Tile ts[4]) 	{ for (int i=0; i<4; i++) ts[i].x--;}
void ts_copy (Tile in[4], Tile out[4]) { for (int i=0; i<4; i++) out[i]=in[i];}
void ts_rotate(Tile ts[4], int midpointTile, int direction)
{
	int a,b,midx,midy;
	midx=ts[midpointTile].x;
	midy=ts[midpointTile].y; 
	if(direction==1 && midpointTile!=0)///anti
	{
		for(int i=0;i<4;++i)
		{
			a=ts[i].x;
			b=ts[i].y;
			ts[i].x=midx-b+midy;
			ts[i].y=midy-midx+a;
		}
	}
	else if(direction == -1 && midpointTile!=0)///clockwise
	{
		for(int i=0;i<4;++i)
		{
			a=ts[i].x;
			b=ts[i].y;
			ts[i].x=midx+b-midy;
			ts[i].y=midy+midx-a;
		}		
	}
}

class ActivePiece
{
	friend class Board;
	Tile ts[4];
	int color;
	int midpointTile;
	Board *b; // container board
	bool moveDown ();
	bool moveLeft ();
	bool moveRight ();
	bool moveBottom();
	bool rotate(int direction);

	public:
	ActivePiece(Board *container);
};

class Board : public Fl_Widget
{
	ActivePiece *p;
	int score;
	Tile newSet[4];
	public:
	Fl_Box *scoreBox; 
	char* scoreLabel;/// to update score
	int temp[xmaxtiles][ymaxtiles]; /// shows non background tile-color if occupied
	Board();
	void setScoreBox(Fl_Box** sb);
	bool isFree(Tile ts[4]);
	bool inBoard(Tile ts[4]);
	bool transferTiles(Tile ts[4], int color);
	void deleteRow(int row);
	bool rowDone(int row);
	bool isRowEmpty(int row);
	void draw();
	int handle(int);
	void set(ActivePiece*);
	int periodic();
	void shiftRows(int row);
};

void timeractions(void *p) { ((Board *)p)->periodic(); }

bool Board::isRowEmpty(int row)
{
	for(int i=0;i<xmaxtiles;++i)
		if(temp[i][row]!=bgcolor)
			return false;
	return true;
}

void Board::shiftRows(int row)
{
	for(int i=row;i>1;--i)
		for(int j=0;j<xmaxtiles;++j)
			temp[j][i]=temp[j][i-1];
			
	for(int i=0;i<xmaxtiles;++i)
		temp[i][0]=bgcolor;
}

int Board::periodic() 
{
	///checks if a row is filled and clears it
	for(int i=0;i<ymaxtiles;++i)
		if(rowDone(i))
		{
			score+=50;
			deleteRow(i);
			shiftRows(i);
		}
	///calculates the lowest coordinate of the active piece
	int ymax=(p->ts[0]).y;
	for(int i=1;i<4;++i) 
		if((p->ts[i]).y>ymax) 
			ymax = (p->ts[i]).y; 
	
	bool flag=true;
	Tile tS[4];
	ts_copy(p->ts,tS);
	ts_down(tS);
	///checks if it can move any more
	for(int j=0;j<4;++j)
		if( temp[(tS[j]).x][(tS[j]).y]!=bgcolor )
		{
			flag=false;
			break;
		}
	///checks if the lower most row is reached	
	if(flag==true && ymax==ymaxtiles-1)
		flag=false;
	///if cant go any further, the colour is transferred to the board
	if(flag==false)
	{
		for(int i=0;i<4;++i)
		{	
			transferTiles(p->ts,p->color);
		}
		///create a new active piece
		delete p;
		p=new ActivePiece(new Board());
	///part for ending the game
	ts_copy(p->ts,newSet);
	ts_down(newSet);
	if( isFree(p->ts)==false || isFree(newSet)==false )
	{
		stringstream strs;
		string s="GAME OVER!";
		strs<<s;
		string temp_str = strs.str();
		strcpy(scoreLabel,"GAME OVER!!\n");
		strcat(scoreLabel,(char*) temp_str.c_str() );
		scoreBox->label(scoreLabel); 
	}
}
	///update the score
	stringstream strs;
	strs<<score;
	string temp_str = strs.str();
	strcpy(scoreLabel,"Score\n");
	strcat(scoreLabel,(char*) temp_str.c_str() );
	scoreBox->label(scoreLabel);
	
	p->moveDown();
	redraw();
	Fl::repeat_timeout (0.5,timeractions,this);
}

Board::Board() : Fl_Widget (0,0,xmaxtiles*tilesize,ymaxtiles*tilesize,"TETRIS")
{
	for(int i=0;i<xmaxtiles;++i)
		for(int j=0;j<ymaxtiles;++j)
			temp[i][j]=bgcolor;
}
	
bool Board::isFree(Tile tS[4])
{ 
	for(int i=0;i<4;++i) 
		if(temp[tS[i].x][tS[i].y] !=bgcolor) return false;
	return true; 
} /// before moving down


bool Board::inBoard(Tile ts[4])
{
	bool flag=true;
	for(int i=0;i<4;++i)
		if(ts[i].x <0 || ts[i].x >=xmaxtiles || ts[i].y<0 || ts[i].x>=ymaxtiles) flag=false;
	return flag;
} /// check boundaries
	
bool Board::transferTiles(Tile ts[4], int color)
{
	for(int i=0;i<4;++i) 
		temp[ts[i].x][ts[i].y]=p->color;
}/// when the piece comes to rest

void Board::deleteRow(int row)
{
	for(int i=0;i<xmaxtiles;++i) 
		temp[i][row]=bgcolor;
}

bool Board::rowDone(int row)
{ 
	for(int i=0;i<xmaxtiles;++i) 
		if(temp[i][row]==bgcolor)
			return false; 
	return true;
}

void Board::draw()
{
	if( isFree(p->ts)==false || isFree(newSet)==false ) return;
	///draw the board
	for(int i=0;i<xmaxtiles;++i)
		for(int j=0;j<ymaxtiles;++j)
			fl_draw_box(FL_BORDER_BOX,i*tilesize,j*tilesize,tilesize,tilesize,temp[i][j]);
	///draw the current piece		
	for(int i=0;i<4;++i)
		fl_draw_box(FL_UP_BOX,p->ts[i].x * tilesize,p->ts[i].y *tilesize,tilesize,tilesize,p->color);
}

void Board::setScoreBox(Fl_Box** sb)
{
	scoreBox = *sb ;
	score=0;
	scoreLabel = (char*) malloc(sizeof(char)*10);
}

int Board::handle(int e)
{
	if(e==8)
	{
		switch(Fl::event_key())
		{
			case 65361 : ///move left
			p->moveLeft();
			break;
			
			case 65363 : ///move right
			p->moveRight();
			break;
			
			case 65362 :///anticlock = 1
			p->rotate(1);
			break;
			
			case 65364 :
			p->rotate(-1);///clock = -1
			break;
			///
			case 32 : ///moving the piece down controlled by spacebar
			p->moveDown();
			break;
			
			case 65307 : exit(1); ///escape key
		}
	}
	redraw();
}/// keypress events

void Board::set(ActivePiece *g)
{ 
	p=g;
}

ActivePiece::ActivePiece(Board *g)
{
	b=g;
	int n=rand();
	n%=7;
	int c=(int)(xmaxtiles/2)-1;

	switch(n)
	{
		case 0:///line
		ts[0].x=c; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=0;
		ts[2].x=c-1; ts[2].y=0;
		ts[3].x=c-2; ts[3].y=0;
		midpointTile =1;
		break;
		
		case 1:///square
		ts[0].x=c; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=0;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c+1; ts[3].y=1;
		midpointTile = 0;
		break;
		
		case 2:///L
		ts[0].x=c-1; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=1;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c-1; ts[3].y=1;
		midpointTile = 2;
		break;
		
		case 3:///inverted L
		ts[0].x=c+1; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=1;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c-1; ts[3].y=1;
		midpointTile = 1;
		break;
		
		case 4:///ladder
		ts[0].x=c; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=0;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c-1; ts[3].y=1;
		midpointTile = 2;
		break;
		
		case 5:///inverted ladder
		ts[0].x=c; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=1;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c-1; ts[3].y=0;
		midpointTile = 2;
		break;
		
		case 6:///tetramino
		ts[0].x=c; ts[0].y=0;
		ts[1].x=c+1; ts[1].y=1;
		ts[2].x=c; ts[2].y=1;
		ts[3].x=c-1; ts[3].y=1;
		midpointTile = 2;
		break;
	}
	Tile tS[4];
	ts_copy(ts,tS);
	ts_down(tS);
	color=rand();
	color%=256;
		if(color == bgcolor)
			color+= 50;
}


bool ActivePiece::moveBottom()
{
	int c=0;
	Tile proposedSet[4];
	ts_copy(ts,proposedSet);
	
	for(int i=ts[midpointTile].y;i<ymaxtiles-1;++i)
	{
		ts_down(proposedSet);
		if( b->inBoard(proposedSet) && b->isFree(proposedSet) && b->isRowEmpty(i) )
			c++; 
	}
	for(int i=1;i<c-1;++i)
		moveDown();
}

bool ActivePiece::moveDown ()
{
	Tile proposedSet[4];
	ts_copy(ts, proposedSet);
	ts_down(proposedSet);
	if(b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, ts);
		return true;
	}
	else return false;
}
 
bool ActivePiece::moveLeft ()
{
	Tile proposedSet[4];
	ts_copy(ts, proposedSet);
	ts_left(proposedSet);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, ts);
		return true;
	}
	else return false;
}

bool ActivePiece::moveRight ()
{
	Tile proposedSet[4];
	ts_copy(ts, proposedSet);
	ts_right(proposedSet);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, ts);
		return true;
	}
	else return false;
}

bool ActivePiece::rotate(int direction)
{
	Tile proposedSet[4];
	ts_copy(ts, proposedSet);
	ts_rotate(proposedSet,midpointTile,direction);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, ts);
		return true;
	}
	else return false;
}

int main()
{
	Fl_Window *window = new Fl_Window(700,600,"Tetris");
	window->color(56);
	
	Fl_Box *box = new Fl_Box(xmaxtiles*tilesize+50,100,150,150,"Score\n0");
	box->box(FL_UP_BOX);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labelsize(20);
    box->labeltype(FL_SHADOW_LABEL);
	Board *bo = new Board();
	ActivePiece *ob = new ActivePiece(bo);
	bo->set(ob);
	bo->setScoreBox(&box);
	window->end();
	window->show();
	Fl::add_timeout(0.5, timeractions,bo);
	return(Fl::run());
}
