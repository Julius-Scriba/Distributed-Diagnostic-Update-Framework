import { useQuery } from '@tanstack/react-query';
import api from '../api';

export interface AuditLog {
  id: string;
  timestamp: string;
  action: string;
  key_id: string | null;
  ip: string | null;
  notes?: string;
}

interface LogsResponse {
  logs: AuditLog[];
}

export default function useAuditLogs() {
  return useQuery({
    queryKey: ['auditLogs'],
    queryFn: async () => {
      const { data } = await api.get<LogsResponse>('/admin/audit_logs');
      return data.logs;
    },
  });
}
