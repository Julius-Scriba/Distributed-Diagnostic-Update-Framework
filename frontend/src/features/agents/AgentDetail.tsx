import { useState } from 'react';
import { useParams } from 'react-router-dom';
import useAgent from '../../api/useAgent';
import useSendCommand from '../../api/useSendCommand';
import { useTemplates, type TemplateEntry } from '../../api/useTemplates';
import useLogs from '../../api/useLogs';
import Spinner from '../../components/Spinner';
import Modal from '../../components/Modal';
import { formatDistanceToNow } from 'date-fns';

export default function AgentDetail() {
  const { uuid } = useParams();
  const { data: agent, isLoading, isError } = useAgent(uuid || null);
  const sendCommand = useSendCommand(uuid || '');
  const { data: templates, isLoading: tLoading, isError: tError } = useTemplates();
  const { data: logs, isLoading: lLoading, isError: lError } = useLogs(uuid || null);

  const [modal, setModal] = useState(false);
  const [cmd, setCmd] = useState('SAFE_MODE');
  const [params, setParams] = useState('{}');
  const [jsonError, setJsonError] = useState('');
  const [templatePreview, setTemplatePreview] = useState<TemplateEntry | null>(null);
  const [expanded, setExpanded] = useState<number | null>(null);
  const [tab, setTab] = useState<'logs' | 'surveillance'>('logs');
  const [image, setImage] = useState<string | null>(null);

  if (isLoading) return <Spinner />;
  if (isError) return <p className="text-red-400">Failed to load agent</p>;
  if (!agent) return <p className="text-red-400">Agent not found</p>;

  const stability = () => {
    const seconds = (Date.now() - new Date(agent.last_seen).getTime()) / 1000;
    if (!agent.online || seconds > 180)
      return <span className="bg-red-600 text-xs px-2 py-1 rounded">🔴 Offline</span>;
    if (seconds <= 30)
      return <span className="bg-green-600 text-xs px-2 py-1 rounded">🟢 Stable</span>;
    if (seconds <= 90)
      return <span className="bg-yellow-500 text-xs px-2 py-1 rounded">🟡 Warning</span>;
    return <span className="bg-orange-500 text-xs px-2 py-1 rounded">🟠 Degraded</span>;
  };

  const submit = () => {
    try {
      const parsed = params ? JSON.parse(params) : {};
      setJsonError('');
      sendCommand.mutate(
        { command: cmd, parameters: parsed },
        { onSuccess: () => setModal(false) }
      );
    } catch {
      setJsonError('Parameters must be valid JSON');
    }
  };

  const sendTemplate = () => {
    if (!templatePreview) return;
    sendCommand.mutate(
      { command: templatePreview.command, parameters: templatePreview.parameters },
      { onSuccess: () => setTemplatePreview(null) }
    );
  };

  return (
    <div className="p-4 text-white space-y-4">
      <h1 className="text-2xl">Agent {agent.uuid}</h1>

      <div className="bg-gray-800 p-4 rounded w-full max-w-md space-y-1">
        <div className="flex justify-between"><span>UUID</span><span className="font-mono">{agent.uuid}</span></div>
        <div className="flex justify-between"><span>Status</span><span>{agent.online ? 'Online' : 'Offline'} </span></div>
        <div className="flex justify-between"><span>Stability</span><span>{stability()}</span></div>
        <div className="flex justify-between"><span>Last Seen</span><span>{formatDistanceToNow(new Date(agent.last_seen), { addSuffix: true })}</span></div>
        <div className="flex justify-between"><span>Build Version</span><span>n/a</span></div>
        <div className="flex justify-between"><span>Registered At</span><span>n/a</span></div>
      </div>

      <div>
        <div className="flex gap-4 mt-4">
          <button
            onClick={() => setTab('logs')}
            className={`px-3 py-1 rounded ${
              tab === 'logs' ? 'bg-gray-700' : 'bg-gray-800 hover:bg-gray-700'
            }`}
          >
            Logs
          </button>
          <button
            onClick={() => setTab('surveillance')}
            className={`px-3 py-1 rounded ${
              tab === 'surveillance'
                ? 'bg-gray-700'
                : 'bg-gray-800 hover:bg-gray-700'
            }`}
          >
            Surveillance
          </button>
        </div>

        {tab === 'logs' && (
          <div>
            {lLoading && <Spinner />}
            {lError && <p className="text-red-400">Failed to load logs</p>}
            {logs && (
              <div className="max-h-64 overflow-y-auto overflow-x-auto mt-2">
                <table className="min-w-full bg-gray-800 text-sm">
                  <thead>
                    <tr>
                      <th className="py-2">Timestamp</th>
                      <th>Type</th>
                      <th>Description</th>
                      <th>Data</th>
                    </tr>
                  </thead>
                  <tbody>
                    {[...logs]
                      .sort(
                        (a, b) =>
                          new Date(b.timestamp).getTime() -
                          new Date(a.timestamp).getTime()
                      )
                      .map((log, idx) => (
                        <tr key={idx} className="text-center">
                          <td className="py-1 px-2">
                            {formatDistanceToNow(new Date(log.timestamp), {
                              addSuffix: true,
                            })}
                          </td>
                          <td>{log.type}</td>
                          <td>{log.description}</td>
                          <td>
                            {log.data && (
                              <div>
                                <pre className="text-left whitespace-pre-wrap">
                                  {expanded === idx
                                    ? log.data
                                    : `${log.data.slice(0, 200)}${
                                        log.data.length > 200 ? '...' : ''
                                      }`}
                                </pre>
                                {log.data.length > 200 && (
                                  <button
                                    className="text-blue-400 underline"
                                    onClick={() =>
                                      setExpanded(expanded === idx ? null : idx)
                                    }
                                  >
                                    {expanded === idx ? 'weniger' : 'mehr anzeigen'}
                                  </button>
                                )}
                              </div>
                            )}
                          </td>
                        </tr>
                      ))}
                  </tbody>
                </table>
              </div>
            )}
          </div>
        )}

        {tab === 'surveillance' && (
          <div>
            {lLoading && <Spinner />}
            {lError && <p className="text-red-400">Failed to load logs</p>}
            {logs && (
              <div className="max-h-64 overflow-y-auto overflow-x-auto mt-2">
                <table className="min-w-full bg-gray-800 text-sm">
                  <thead>
                    <tr>
                      <th className="py-2">Timestamp</th>
                      <th>Person</th>
                      <th>Image</th>
                      <th>Data</th>
                    </tr>
                  </thead>
                  <tbody>
                    {logs
                      .filter(l => l.type === 'Surveillance')
                      .sort(
                        (a, b) =>
                          new Date(b.timestamp).getTime() -
                          new Date(a.timestamp).getTime()
                      )
                      .map((log, idx) => {
                        let payload: any = {};
                        try {
                          payload = JSON.parse(log.data || '{}');
                        } catch {
                          payload = {};
                        }
                        return (
                          <tr key={idx} className="text-center">
                            <td className="py-1 px-2">
                              {formatDistanceToNow(new Date(log.timestamp), {
                                addSuffix: true,
                              })}
                            </td>
                            <td>{payload.name || 'n/a'}</td>
                            <td>
                              {payload.image_url && (
                                <img
                                  src={payload.image_url}
                                  onClick={() => setImage(payload.image_url)}
                                  className="w-16 h-16 object-cover cursor-pointer mx-auto"
                                />
                              )}
                            </td>
                            <td>
                              {log.data && (
                                <div>
                                  <pre className="text-left whitespace-pre-wrap">
                                    {expanded === idx
                                      ? log.data
                                      : `${log.data.slice(0, 200)}${
                                          log.data.length > 200 ? '...' : ''
                                        }`}
                                  </pre>
                                  {log.data.length > 200 && (
                                    <button
                                      className="text-blue-400 underline"
                                      onClick={() =>
                                        setExpanded(expanded === idx ? null : idx)
                                      }
                                    >
                                      {expanded === idx
                                        ? 'weniger'
                                        : 'mehr anzeigen'}
                                    </button>
                                  )}
                                </div>
                              )}
                            </td>
                          </tr>
                        );
                      })}
                  </tbody>
                </table>
              </div>
            )}
          </div>
        )}
      </div>

      <button
        className="bg-blue-600 hover:bg-blue-700 py-2 px-4 rounded"
        onClick={() => setModal(true)}
      >
        Send Command
      </button>

      <div>
        <h2 className="text-xl mb-2 mt-4">Command Templates</h2>
        {tLoading && <Spinner />}
        {tError && <p className="text-red-400">Failed to load templates</p>}
        {templates && (
          <div className="flex flex-wrap gap-2">
            {templates.map(t => (
              <button
                key={t.template_id}
                className="bg-gray-700 hover:bg-gray-600 px-3 py-1 rounded"
                onClick={() => setTemplatePreview(t)}
              >
                {t.name}
              </button>
            ))}
          </div>
        )}
      </div>

      <Modal open={!!templatePreview} onClose={() => setTemplatePreview(null)}>
        {templatePreview && (
          <div>
            <h2 className="text-xl mb-2">{templatePreview.name}</h2>
            <pre className="bg-gray-700 p-2 rounded text-sm mb-2 whitespace-pre-wrap">
              {JSON.stringify(
                { command: templatePreview.command, parameters: templatePreview.parameters },
                null,
                2
              )}
            </pre>
            <button
              className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded"
              onClick={sendTemplate}
              disabled={sendCommand.isPending}
            >
              {sendCommand.isPending ? (
                <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mx-auto"></div>
              ) : (
                'Senden'
              )}
            </button>
            {sendCommand.isSuccess && (
              <p className="text-green-400 mt-2 text-center">Command queued</p>
            )}
            {sendCommand.isError && (
              <p className="text-red-400 mt-2 text-center">Error</p>
            )}
          </div>
        )}
      </Modal>

      <Modal open={!!image} onClose={() => setImage(null)}>
        {image && (
          <img src={image} className="max-w-full max-h-screen" />
        )}
      </Modal>

      <Modal open={modal} onClose={() => setModal(false)}>
        <h2 className="text-xl mb-2">Send Command</h2>
        <input
          className="w-full bg-gray-700 p-2 rounded mb-2"
          value={cmd}
          onChange={(e) => setCmd(e.target.value)}
          placeholder="Command"
        />
        <textarea
          className="w-full bg-gray-700 p-2 rounded mb-2"
          rows={3}
          value={params}
          onChange={(e) => setParams(e.target.value)}
        ></textarea>
        {jsonError && <p className="text-red-400 text-sm mb-2">{jsonError}</p>}
        <button
          className="w-full bg-blue-600 hover:bg-blue-700 py-2 rounded"
          onClick={submit}
          disabled={sendCommand.isPending}
        >
          {sendCommand.isPending ? (
            <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin mx-auto"></div>
          ) : (
            'Send'
          )}
        </button>
        {sendCommand.isSuccess && (
          <p className="text-green-400 mt-2 text-center">Command queued</p>
        )}
        {sendCommand.isError && (
          <p className="text-red-400 mt-2 text-center">Error</p>
        )}
      </Modal>
    </div>
  );
}
