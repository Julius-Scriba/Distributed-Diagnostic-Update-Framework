export default function Header({ onLogout }: { onLogout: () => void }) {
  return (
    <header className="bg-[#1a1a1a] shadow px-6 py-4 flex items-center justify-between sticky top-0 z-10 text-cyan-400">
      <h1 className="text-lg font-semibold">ULTSPY Dashboard</h1>
      <button
        className="text-sm text-neonBlue hover:text-white"
        onClick={onLogout}
      >
        Logout
      </button>
    </header>
  );
}
