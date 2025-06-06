import { Outlet } from 'react-router-dom';
import Header from './Header';
import Sidebar from './Sidebar';

interface Props {
  onLogout: () => void;
  error: string | null;
  clearError: () => void;
}
export default function Layout({ onLogout, error, clearError }: Props) {
  return (
    <div className="flex h-screen overflow-hidden bg-[#121212]">
      <Sidebar />
      <div className="flex-1 flex flex-col overflow-hidden">
        <Header onLogout={onLogout} />
        {error && (
          <div className="bg-red-600 text-white px-4 py-2 text-center">
            <span>{error}</span>
            <button className="ml-4 underline" onClick={clearError}>X</button>
          </div>
        )}
        <main className="flex-1 p-4 overflow-y-auto bg-[#181818] text-gray-200">
          <Outlet />
        </main>
      </div>
    </div>
  );
}
