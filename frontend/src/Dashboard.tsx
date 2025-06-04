import { useQuery } from '@tanstack/react-query';
import api from './api';

interface Agent {
  uuid: string;
  online: boolean;
  last_seen: string;
}

export default function Dashboard() {
  const { data } = useQuery({
    queryKey: ['agents'],
    queryFn: async () => (await api.get<Agent[]>('/admin/agents')).data,
  });

  return (
    <div className="p-4 text-white">
      <h1 className="text-2xl mb-4">Agents</h1>
      <table className="min-w-full bg-gray-800">
        <thead>
          <tr>
            <th className="py-2">UUID</th>
            <th>Status</th>
            <th>Last Seen</th>
          </tr>
        </thead>
        <tbody>
          {data?.map(a => (
            <tr key={a.uuid} className="text-center">
              <td className="py-1 px-2">{a.uuid}</td>
              <td>{a.online ? 'Online' : 'Offline'}</td>
              <td>{a.last_seen}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
