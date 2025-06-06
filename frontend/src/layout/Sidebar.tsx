import { NavLink } from 'react-router-dom';

const links = [
  { to: '/', label: 'Dashboard' },
  { to: '/agents', label: 'Agents' },
  { to: '/commands', label: 'Commands' },
  { to: '/templates', label: 'Templates' },
  { to: '/surveillance', label: 'Surveillance' },
  { to: '/logs', label: 'Logs' },
  { to: '/settings', label: 'Settings' },
];

export default function Sidebar() {
  return (
    <aside className="w-48 bg-gray-800 text-white flex flex-col p-4 space-y-2 sticky top-0 h-screen">
      {links.map(link => (
        <NavLink
          key={link.to}
          to={link.to}
          className={({ isActive }) =>
            `px-2 py-1 rounded transition-colors hover:bg-gray-700 hover:text-white ${isActive ? 'bg-gray-700 text-blue-300' : ''}`
          }
        >
          {link.label}
        </NavLink>
      ))}
    </aside>
  );
}
