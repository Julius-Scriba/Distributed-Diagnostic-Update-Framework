import { NavLink } from 'react-router-dom';

const links = [
  { to: '/', label: 'Dashboard' },
  { to: '/agents', label: 'Agents' },
  { to: '/commands', label: 'Commands' },
  { to: '/templates', label: 'Templates' },
  { to: '/logs', label: 'Logs' },
  { to: '/settings', label: 'Settings' },
];

export default function Sidebar() {
  return (
    <aside className="w-48 bg-gray-800 text-white flex flex-col p-4 space-y-2">
      {links.map(link => (
        <NavLink
          key={link.to}
          to={link.to}
          className={({ isActive }) =>
            `px-2 py-1 rounded hover:bg-gray-700 ${isActive ? 'bg-gray-700' : ''}`
          }
        >
          {link.label}
        </NavLink>
      ))}
    </aside>
  );
}
