import { Outlet } from 'react-router-dom';
import Header from './Header';
import Sidebar from './Sidebar';

export default function Layout({ onLogout }: { onLogout: () => void }) {
  return (
    <div className="flex h-screen text-gray-800">
      <Sidebar />
      <div className="flex-1 flex flex-col">
        <Header onLogout={onLogout} />
        <main className="flex-1 p-4 overflow-auto bg-gray-100">
          <Outlet />
        </main>
      </div>
    </div>
  );
}
