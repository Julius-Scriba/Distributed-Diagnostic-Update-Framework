import useAgents from '../api/useAgents';
import Spinner from '../components/Spinner';
import { Link } from 'react-router-dom';

export default function Dashboard() {
  const { data, isLoading, isError } = useAgents();
  return (
    <div className="space-y-4">
      <h1 className="text-2xl text-neonBlue mb-4">Dashboard</h1>
      {isLoading && <Spinner />}
      {isError && <p className="text-red-400">Fehler beim Laden</p>}
      {data && (
        <div>
          <p className="mb-2">Insgesamt {data.length} Agents registriert.</p>
          <div className="overflow-x-auto">
          <table className="min-w-full bg-[#232323] text-sm">
            <thead>
              <tr>
                <th className="py-2">UUID</th>
                <th>Status</th>
              </tr>
            </thead>
            <tbody>
              {data.slice(0,5).map(a => (
                <tr key={a.uuid} className="text-center">
                  <td className="py-1 px-2">
                    <Link className="text-neonBlue hover:underline" to={`/agents/${a.uuid}`}>{a.uuid}</Link>
                  </td>
                  <td>
                    <span className={`inline-block w-2 h-2 rounded-full mr-1 ${a.online ? 'bg-green-500' : 'bg-red-500'}`}></span>
                    {a.online ? 'Online' : 'Offline'}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
          </div>
        </div>
      )}
    </div>
  );
}
