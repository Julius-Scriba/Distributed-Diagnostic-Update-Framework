import { Link } from 'react-router-dom';
import useAgents from '../api/useAgents';
import Spinner from '../components/Spinner';
import { formatDistanceToNow } from 'date-fns';

export default function Agents() {
  const { data, isLoading, isError } = useAgents();

  return (
    <div className="p-4 space-y-4">
      <h1 className="text-2xl mb-4 text-neonBlue">Agents</h1>
      {isLoading && <Spinner />}
      {isError && <p className="text-red-400">Failed to load agents</p>}
      {data && (
        <div className="overflow-x-auto">
        <table className="min-w-full bg-[#232323] text-sm">
          <thead>
            <tr>
              <th className="py-2">UUID</th>
              <th>Status</th>
              <th>Last Seen</th>
              <th>Stability</th>
            </tr>
          </thead>
          <tbody>
            {[...data].sort((a,b)=>a.uuid.localeCompare(b.uuid)).map(a => (
              <tr key={a.uuid} className="text-center">
                <td className="py-1 px-2">
                  <Link className="text-neonBlue hover:underline" to={`/agents/${a.uuid}`}>{a.uuid}</Link>
                </td>
                <td>
                  <span
                    className={`inline-block w-2 h-2 rounded-full mr-1 ${a.online ? 'bg-green-500' : 'bg-red-500'}`}
                  ></span>
                  {a.online ? 'Online' : 'Offline'}
                </td>
                <td>{formatDistanceToNow(new Date(a.last_seen), { addSuffix: true })}</td>
                <td>
                  {(() => {
                    const seconds =
                      (Date.now() - new Date(a.last_seen).getTime()) / 1000;
                    if (!a.online || seconds > 180)
                      return <span className="bg-red-600 text-xs px-2 py-1 rounded">🔴 Offline</span>;
                    if (seconds <= 30)
                      return <span className="bg-green-600 text-xs px-2 py-1 rounded">🟢 Stable</span>;
                    if (seconds <= 90)
                      return <span className="bg-yellow-500 text-xs px-2 py-1 rounded">🟡 Warning</span>;
                    return <span className="bg-orange-500 text-xs px-2 py-1 rounded">🟠 Degraded</span>;
                  })()}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        </div>
      )}
    </div>
  );
}
