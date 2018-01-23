package testP;

public class tester implements Runnable{
	public String threadName;
	public Thread thread;
	public String task = "credit";
	public SafeAccount account;
	
	public tester(String name, String tas, SafeAccount account){
		threadName = name;
		task = tas;
		System.out.println("Creating thread " + threadName + " with task " + task);
		this.account = account;
	}
	
	public static void main(String[] args) {
		SafeAccount account = new SafeAccount(1000, 10);
		tester test1 = new tester("CreditThread", "credit", account);
		tester test2 = new tester("DebitThread", "debit", account);
		
		test1.start();
		test2.start();
	}

	@Override
	public void run() {
		if(task.equals("credit")){
			try {
				account.credit(1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		if(task.equals("debit")){
			try {
				account.debit(10);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	public void start(){
		System.out.println("Starting thread " + threadName);
		if (thread == null){
			thread = new Thread (this, threadName);
			thread.start();
		}
		
	}
}
