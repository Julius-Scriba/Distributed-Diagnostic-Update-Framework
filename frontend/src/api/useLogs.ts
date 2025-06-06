import { useQuery } from '@tanstack/react-query';
import api from '../api';

export interface LogItem {
  timestamp: string;
  type: string;
  description: string;
  data?: string;
}

interface LogsResponse {
  logs: LogItem[];
}

export default function useLogs(uuid: string | null) {
  return useQuery({
    queryKey: ['logs', uuid],
    queryFn: async () => {
      const { data } = await api.get<LogsResponse>(`/admin/logs/${uuid}`);
      return data.logs;
    },
    enabled: !!uuid,
  });
}
