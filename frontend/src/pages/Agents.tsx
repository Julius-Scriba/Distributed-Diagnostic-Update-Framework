import { Link } from 'react-router-dom';
import useAgents from '../api/useAgents';
import Spinner from '../components/Spinner';

export default function Agents() {
  const { data, isLoading, isError } = useAgents();

  return (
    <div className="p-4 text-white">
      <h1 className="text-2xl mb-4">Agents</h1>
      {isLoading && <Spinner />}
      {isError && <p className="text-red-400">Failed to load agents</p>}
      {data && (
        <table className="min-w-full bg-gray-800">
          <thead>
            <tr>
              <th className="py-2">UUID</th>
              <th>Status</th>
              <th>Last Seen</th>
            </tr>
          </thead>
          <tbody>
            {data.map(a => (
              <tr key={a.uuid} className="text-center">
                <td className="py-1 px-2">
                  <Link className="text-blue-400 hover:underline" to={`/agents/${a.uuid}`}>{a.uuid}</Link>
                </td>
                <td>
                  <span
                    className={`inline-block w-2 h-2 rounded-full mr-1 ${a.online ? 'bg-green-500' : 'bg-red-500'}`}
                  ></span>
                  {a.online ? 'Online' : 'Offline'}
                </td>
                <td>{a.last_seen}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </div>
  );
}
