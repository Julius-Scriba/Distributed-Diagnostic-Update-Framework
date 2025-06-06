import { NavLink } from 'react-router-dom';
import {
  LayoutDashboard,
  Users,
  TerminalSquare,
  List,
  Camera,
  FileText,
  Settings as SettingsIcon,
  Key,
  ListChecks,
} from 'lucide-react';

const links = [
  { to: '/', label: 'Dashboard', icon: LayoutDashboard },
  { to: '/agents', label: 'Agents', icon: Users },
  { to: '/commands', label: 'Commands', icon: TerminalSquare },
  { to: '/templates', label: 'Templates', icon: List },
  { to: '/surveillance', label: 'Surveillance', icon: Camera },
  { to: '/logs', label: 'Logs', icon: FileText },
  { to: '/settings', label: 'Settings', icon: SettingsIcon },
  { to: '/apikeys', label: 'API Keys', icon: Key },
  { to: '/audit', label: 'Audit Log', icon: ListChecks },
];

export default function Sidebar() {
  return (
    <aside className="w-48 bg-[#1a1a1a] text-gray-200 flex flex-col p-4 space-y-2 sticky top-0 h-screen">
      {links.map(({ to, label, icon: Icon }) => (
        <NavLink
          key={to}
          to={to}
          className={({ isActive }) =>
            `flex items-center space-x-2 px-2 py-2 rounded transition-colors hover:bg-[#232323] ${isActive ? 'bg-[#232323] text-neonBlue' : ''}`
          }
        >
          <Icon className="w-4 h-4" />
          <span>{label}</span>
        </NavLink>
      ))}
    </aside>
  );
}
