// shims.cpp : Defines the entry point for the console application.
//

#include <iostream.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	float e1,e2,e3,e4,i1,i2,i3,i4;
	float se1,se2,se3,se4,si1,si2,si3,si4;
	int coldtest=1;
	char test;

	cout<<"Did you measure gaps in a (c)old or (h)ot motor?"<<endl;
	cin>>test;
	if(test=='h'||test=='H')
	{
		coldtest=0;
		cout<<"Hot it is"<<endl<<endl;
	}
	else
		cout<<"Cold it is"<<endl<<endl;

	cout<<"Enter valve gaps"<<endl;
	cout<<"Cyl 1 exhaust: ";
	cin>>e1;
	cout<<"Cyl 1  intake: ";
	cin>>i1;
	cout<<"Cyl 2 exhaust: ";
	cin>>e2;
	cout<<"Cyl 2  intake: ";
	cin>>i2;
	cout<<"Cyl 3 exhaust: ";
	cin>>e3;
	cout<<"Cyl 3  intake: ";
	cin>>i3;
	cout<<"Cyl 4 exhaust: ";
	cin>>e4;
	cout<<"Cyl 4  intake: ";
	cin>>i4;

	cout<<"Enter valve shim sizes"<<endl;
    cout<<"Cyl 1 exhaust: ";
	cin>>se1;
	cout<<"Cyl 1  intake: ";
	cin>>si1;
	cout<<"Cyl 2 exhaust: ";
	cin>>se2;
	cout<<"Cyl 2  intake: ";
	cin>>si2;
	cout<<"Cyl 3 exhaust: ";
	cin>>se3;
	cout<<"Cyl 3  intake: ";
	cin>>si3;
	cout<<"Cyl 4 exhaust: ";
	cin>>se4;
	cout<<"Cyl 4  intake: ";
	cin>>si4;

	e1=e1/.03937;
	e2=e2/.03937;
	e3=e3/.03937;
	e4=e4/.03937;
	i1=i1/.03937;
	i2=i2/.03937;
	i3=i3/.03937;
	i4=i4/.03937;

	i1=i1-.203;
	si1=si1+i1;
	i2=i2-.203;
	si2=si2+i2;
	i3=i3-.203;
	si3=si3+i3;
	i4=i4-.203;
	si4=si4+i4;

	e1=e1-.406;
	se1=se1+e1;
	e2=e2-.406;
	se2=se2+e2;
	e3=e3-.406;
	se3=se3+e3;
	e4=e4-.406;
	se4=se4+e4;

	cout<<endl<<"The desired shims are (+- .05mm)"<<endl;
	cout<<"Cyl 1 exhaust: "	<<se1<<endl;
	cout<<"Cyl 1  intake: "<<si1<<endl;
	cout<<"Cyl 2 exhaust: "<<se2<<endl;
	cout<<"Cyl 2  intake: "<<si2<<endl;
	cout<<"Cyl 3 exhaust: "<<se3<<endl;
	cout<<"Cyl 3  intake: "<<si3<<endl;
	cout<<"Cyl 4 exhaust: "<<se4<<endl;
	cout<<"Cyl 4  intake: "<<si4<<endl;
	return 0;
}
