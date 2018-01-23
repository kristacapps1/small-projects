
public class SafeAccount {
	public int balance;
    int debit;
    int deposit;
    int n;

	public static void main(String[] args) {
		int n = 0;
        int balance = 0;
		SafeAccount newAccount = new SafeAccount(n,balance);
	}
    //Constructor
	public SafeAccount(int n, int bal) {
		balance = bal;
        debit = 0;
        deposit = 0;
        this.n = n;
        
	}
	//Deposit amount into safeAccount
	synchronized public void credit(int amount){
        while(balance > n)
            deposit.wait()
		//if(balance < n){
        balance += amount;
        debit.notify();
		//}
	}
	//Withdraw amount from safeAccount
	synchronized public void debit(int amount){
		if((balance - debit) < 0)
			debit.wait()
        balance -= amount;
        deposit.notifyAll()
	}
}
