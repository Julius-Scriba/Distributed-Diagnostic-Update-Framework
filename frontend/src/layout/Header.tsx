export default function Header({ onLogout }: { onLogout: () => void }) {
  return (
    <header className="bg-white shadow px-6 py-4 flex items-center justify-between">
      <h1 className="text-lg font-semibold">ULTSPY Dashboard</h1>
      <button
        className="text-sm text-blue-600 hover:underline"
        onClick={onLogout}
      >
        Logout
      </button>
    </header>
  );
}
