package bulletbloom.shop;

/**
 * Tracks spendable money and purchase transactions.
 */
public final class Wallet {
    private int balance;

    /**
     * Creates an empty wallet.
     */
    public Wallet() {
    }

    /**
     * Gets the current balance.
     *
     * @return available money
     */
    public int getBalance() {
        return balance;
    }

    /**
     * Restores the wallet balance.
     *
     * @param balance new balance
     */
    public void setBalance(int balance) {
        this.balance = Math.max(0, balance);
    }

    /**
     * Adds money to the wallet.
     *
     * @param amount amount to add
     */
    public void add(int amount) {
        balance = Math.max(0, balance + amount);
    }

    /**
     * Attempts to spend money from the wallet.
     *
     * @param amount amount to spend
     * @return {@code true} when the transaction succeeded
     */
    public boolean spend(int amount) {
        if (amount < 0 || balance < amount) {
            return false;
        }
        balance -= amount;
        return true;
    }
}
