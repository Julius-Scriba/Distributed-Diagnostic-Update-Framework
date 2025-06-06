import { formatDistanceToNow } from 'date-fns';
import useAuditLogs from '../api/useAuditLogs';
import Spinner from '../components/Spinner';

export default function AuditLog() {
  const { data, isLoading, error } = useAuditLogs();
  if (isLoading) return <Spinner />;
  if (error) return <div className="text-red-600">Error loading logs</div>;
  return (
    <div className="space-y-4">
      <h1 className="text-xl font-bold text-neonBlue">Audit Log</h1>
      <table className="min-w-full bg-[#232323]">
        <thead>
          <tr>
            <th className="p-2 text-left">Time</th>
            <th className="p-2 text-left">Action</th>
            <th className="p-2 text-left">Key ID</th>
            <th className="p-2 text-left">IP</th>
          </tr>
        </thead>
        <tbody>
          {data?.map(l => (
            <tr key={l.id} className="border-t">
              <td className="p-2">{formatDistanceToNow(new Date(l.timestamp), { addSuffix: true })}</td>
              <td className="p-2">{l.action}</td>
              <td className="p-2">{l.key_id ?? '-'}</td>
              <td className="p-2">{l.ip ?? '-'}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
