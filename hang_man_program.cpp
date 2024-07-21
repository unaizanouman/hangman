#include<iostream>
#include<ctime>
#include<string.h>
#include<cstdlib>
using namespace std;
int tries=3;
string message="You have 3 chances";

void hang_man(char);
int check_guess(char, string, string&);

main()
{
	char letter;
	string month;
	string months[]={"january","february","march","april","may","june","july","august",
	"september","october","november","december"};
	srand(time(NULL));
	int month_number=rand()%12;
	month=months[month_number];
	string hide_month(month.length(),'X');
	system("cls");
	
	while(tries!=0)
	{
		hang_man('n');
		cout<<"\t\t\t\tlife: "<<tries<<endl;
		cout<<"\t\t\t\t"<<hide_month<<endl;
		cout<<"\t\t\t\tGuess a letter: ";
		cin>>letter;
		system("cls");
		
		if(check_guess(letter, month, hide_month)==0)
		{
			message="Incorrect letter!";
			tries--;
		}
		else
		{
			message="Nice! Correct guess!";
			
		}
		if (hide_month==month)
		{
			message="You are free!";
            hang_man('f');
            cout<<"\t\t\t\tlife: "<<tries<<endl;
            cout<<"\t\t\t\tThe month is: "<<month<<endl;
			break;
		}
	}
	if(tries==0)
	{
		message="You are hanged!";
		hang_man('h');
		cout<<"\t\t\t\tlife: "<<tries<<endl;
		cout<<"\t\t\t\tThe month is: "<<month<<endl;
	}
	
}
int check_guess(char guess, string real_month, string &hidden_month)
{
	int matches=0;
	int len=real_month.length();
	for(int i=0; i<len; i++)
	{
		if (guess==hidden_month[i])
		return 0;
		if(guess==real_month[i])
		{
			hidden_month[i]==guess;//revealing letters
			matches++;
		}
	}
	return matches;
}
void hang_man(char state)
{
	string head_string="|";
	string stage="=";
	if(state=='f')
	head_string=" ";
	else if(state=='h')
	stage=" ";
	
	cout<<"\t\t\t\t____________________"<<endl;
	cout<<"\t\t\t\t         "<<head_string<<"         |"<<endl;
	cout<<"\t\t\t\t         O         |"<<endl;
	cout<<"\t\t\t\t        / \\        |"<<"\t          "<<message<<endl;
	cout<<"\t\t\t\t         |         |"<<"\t       /"<<endl;
	cout<<"\t\t\t\t        / \\        |            O  "<<endl;
	cout<<"\t\t\t\t     =="<<stage<<stage<<stage<<stage<<stage<<"==     |           / \\"<<endl;
	cout<<"\t\t\t\t     |       |     |            |"<<endl;
	cout<<"\t\t\t\t     |       |     |           /  \\"<<endl;
	cout<<"\t\t\t\t=========================================="<<endl;
		
}

