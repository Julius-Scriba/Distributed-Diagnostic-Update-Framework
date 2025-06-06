import useSurveillance from '../api/useSurveillance';
import Spinner from '../components/Spinner';
import { formatDistanceToNow } from 'date-fns';
import { useState } from 'react';

export default function Surveillance() {
  const { data, isLoading, isError } = useSurveillance();
  const [image, setImage] = useState<string | null>(null);

  return (
    <div className="p-4 space-y-4">
      <h1 className="text-2xl mb-2 text-neonBlue">Surveillance</h1>
      {isLoading && <Spinner />}
      {isError && <p className="text-red-400">Failed to load surveillance data</p>}
      {data && (
        <div className="overflow-x-auto">
          <table className="min-w-full bg-[#232323] text-sm">
            <thead>
              <tr>
                <th className="py-2">Agent</th>
                <th>Timestamp</th>
                <th>Person</th>
                <th>Image</th>
              </tr>
            </thead>
            <tbody>
              {data.map((entry, idx) => (
                <tr key={idx} className="text-center">
                  <td className="py-1 px-2">{entry.agent_uuid}</td>
                  <td>{formatDistanceToNow(new Date(entry.timestamp), { addSuffix: true })}</td>
                  <td>{entry.name || 'n/a'}</td>
                  <td>
                    {entry.image_url && (
                      <img
                        src={entry.image_url}
                        onClick={() => setImage(entry.image_url!)}
                        className="w-16 h-16 object-cover cursor-pointer mx-auto"
                      />
                    )}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}

      {image && (
        <div
          className="fixed inset-0 bg-black bg-opacity-75 flex items-center justify-center"
          onClick={() => setImage(null)}
        >
          <img src={image} className="max-w-full max-h-full" />
        </div>
      )}
    </div>
  );
}
